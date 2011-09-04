#ifndef COT_MODEL_H
#define COT_MODEL_H

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/tss.hpp>
#include "cot/connection.h"
#include "cot/list.h"
#include "cot/type.h"

template<class Subclass>
class Model
{
    public:
        static const int fieldCount = Length<typename Subclass::fieldList>::value;
        static const int totalFieldSize = MemCount<typename Subclass::typeList>::value;
        
        virtual ~Model() {}
        
        void save() {
            if (resource.get() == NULL) {
                resource.reset(new Resource());
            }

            if (!resource->initialized) {
            
                // 1. INSERT
                
                // 1.1. Get the INSERT query string.
                std::string insertQueryStr = std::string("INSERT INTO ") + Subclass::stringify() + " VALUES ";
                insertQueryStr += "(";
                for (int i = 0; i < fieldCount; ++i) {
                    if (i == 0) {
                        insertQueryStr += "NULL"; // id has auto_increment property
                    } else {
                        insertQueryStr += ", ?";
                    }
                }
                insertQueryStr += ")";
                
                // 1.2. Prepare the INSERT statement.
                resource->insertStatement = mysql_stmt_init(Connection::connection());
                char * insertQueryCStr = new char[insertQueryStr.length() + 1];
                std::strncpy(insertQueryCStr, insertQueryStr.c_str(), insertQueryStr.length());
                if (mysql_stmt_prepare(resource->insertStatement,
                            insertQueryCStr,
                            insertQueryStr.length()) != 0) {
                    delete[] insertQueryCStr;
                    throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                            mysql_stmt_error(resource->insertStatement));
                }
                delete[] insertQueryCStr;
                
                // 1.3. Prepare MYSQL_BIND structures for the INSERT query.
                resource->insertMem = new uint8_t[totalFieldSize];
                resource->insertBindMem = new MYSQL_BIND[fieldCount];
                resource->insertDataLength = new int[fieldCount];

                InsertBindsPrepareProcedureParam ibpp_param;
                ibpp_param.index = fieldCount - 1;
                ibpp_param.offset = 0;
                ibpp_param.resource = resource.get();
                Exec<typename Subclass::typeList, InsertBindsPrepareProcedure>::exec(&ibpp_param);
                if (mysql_stmt_bind_param(resource->insertStatement, resource->insertBindMem + 1) != 0) { // except the id
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                            mysql_stmt_error(resource->insertStatement));
                }
                
                // 2. UPDATE
                
                // 2.1. Get the UPDATE query string.
                std::vector<std::string> fieldNames;
                Exec<typename Subclass::fieldList, GetFieldNamesProcedure>::exec(&fieldNames);

                std::string updateQueryStr = std::string("UPDATE ") + Subclass::stringify() + " SET ";
                for (int i = fieldCount - 2; i >= 0; --i) { // except the id, in normal order
                    updateQueryStr += fieldNames[i] + " = ?";
                    if (i != 0) { updateQueryStr += ", "; }
                }
                updateQueryStr += " WHERE id = ?";
                
                // 2.2. Prepare the UPDATE statement
                resource->updateStatement = mysql_stmt_init(Connection::connection());
                char * updateQueryCStr = new char[updateQueryStr.length() + 1];
                std::strncpy(updateQueryCStr, updateQueryStr.c_str(), updateQueryStr.length());
                if (mysql_stmt_prepare(resource->updateStatement,
                            updateQueryCStr,
                            updateQueryStr.length()) != 0) {
                    delete[] updateQueryCStr;
                    throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                            mysql_stmt_error(resource->updateStatement));
                }
                delete[] updateQueryCStr;
                
                // 2.3. Prepare MYSQL_BIND structures for the UPDATE query.
                resource->updateMem = new uint8_t[totalFieldSize];
                resource->updateBindMem = new MYSQL_BIND[fieldCount + 1];
                resource->updateDataLength = new int[fieldCount];

                UpdateBindsPrepareProcedureParam ubpp_param;
                ubpp_param.index = fieldCount - 1;
                ubpp_param.offset = 0;
                ubpp_param.resource = resource.get();
                Exec<typename Subclass::typeList, UpdateBindsPrepareProcedure>::exec(&ubpp_param);
                std::memcpy(resource->updateBindMem + fieldCount,
                        resource->updateBindMem,
                        sizeof(MYSQL_BIND));
                if (mysql_stmt_bind_param(resource->updateStatement, resource->updateBindMem + 1) != 0) { // moved id
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                            mysql_stmt_error(resource->updateStatement));
                }
                
                resource->initialized = true;
            }
            
            if (Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this)) == 0) {
                // INSERT
                InsertParamCopyProcedureParam ipcp_param;
                ipcp_param.index = fieldCount - 1;
                ipcp_param.offset = 0;
                ipcp_param.model = reinterpret_cast<Subclass *>(this);
                ipcp_param.resource = resource.get();
                Exec<typename Subclass::fieldList, InsertParamCopyProcedure>::exec(&ipcp_param);
                
                if (mysql_stmt_execute(resource->insertStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(resource->insertStatement));
                }
                
                Subclass::_id_::setCppValue(
                        reinterpret_cast<Subclass *>(this),
                        (int)mysql_insert_id(Connection::connection()));
            } else {
                // UPDATE
                UpdateParamCopyProcedureParam upcp_param;
                upcp_param.index = fieldCount - 1;
                upcp_param.offset = 0;
                upcp_param.model = reinterpret_cast<Subclass *>(this);
                upcp_param.resource = resource.get();
                Exec<typename Subclass::fieldList, UpdateParamCopyProcedure>::exec(&upcp_param);
                std::memcpy(resource->updateBindMem + fieldCount, resource->updateBindMem, sizeof(MYSQL_BIND));
                
                if (mysql_stmt_execute(resource->updateStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(resource->updateStatement));
                }
            }
        }
        
        void remove() {
            if (resource.get() == NULL) {
                resource.reset(new Resource());
            }

            if (Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this)) != 0) {
                if (!resource->deleteInitialized) {
                    // NOTE: the query string is always the same.
                    
                    // 1. Prepare the statement.
                    resource->deleteStatement = mysql_stmt_init(Connection::connection());
                    std::string deleteQueryStr = \
                            std::string("DELETE FROM ") + Subclass::stringify() + " WHERE id = ?";
                    char * deleteQueryCStr = new char[deleteQueryStr.length() + 1];
                    std::strncpy(deleteQueryCStr, deleteQueryStr.c_str(), deleteQueryStr.length());
                    if (mysql_stmt_prepare(resource->deleteStatement,
                                deleteQueryCStr,
                                deleteQueryStr.length()) != 0) {
                        delete[] deleteQueryCStr;
                        throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                                mysql_stmt_error(resource->deleteStatement));
                    }
                    delete[] deleteQueryCStr;
                    
                    // 2. Bind the parameter.
                    resource->deleteMem = new uint8_t[IntValue::max_length];
                    resource->deleteBindMem = new MYSQL_BIND[1];
                    resource->deleteDataLength = new int[1];
                    boost::scoped_ptr<AbstractValue> value(new IntValue);
                    value->bind(resource->deleteMem, value->get_max_length());
                    resource->deleteDataLength[0] = value->get_data_length();
                    prepare(resource->deleteBindMem, value.get(), resource->deleteDataLength, true);
                    if (mysql_stmt_bind_param(resource->deleteStatement, resource->deleteBindMem) != 0) {
                        throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                                mysql_stmt_error(resource->deleteStatement));
                    }
                    
                    resource->deleteInitialized = true;
                }
                
                IntValue::cpptype cppvalue = \
                        Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this));
                boost::scoped_ptr<AbstractValue> value(IntValue::from_cpptype(cppvalue));
                value->bind(resource->deleteMem, value->get_max_length());
                resource->deleteDataLength[0] = value->get_data_length();
                prepare(resource->deleteBindMem, value.get(), resource->deleteDataLength, true);
                
                if (mysql_stmt_execute(resource->deleteStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(resource->deleteStatement));
                }
                
                Subclass::_id_::setCppValue(reinterpret_cast<Subclass *>(this), 0);
            }
        }
        
    protected:

        class Resource
        {
            public:
                std::string insertQueryStr;
                std::string updateQueryStr;

                bool initialized;
                bool deleteInitialized;

                MYSQL_STMT * insertStatement;
                uint8_t * insertMem;
                MYSQL_BIND * insertBindMem;
                int * insertDataLength;

                MYSQL_STMT * updateStatement;
                uint8_t * updateMem;
                MYSQL_BIND * updateBindMem;
                int * updateDataLength;

                MYSQL_STMT * deleteStatement;
                uint8_t * deleteMem;
                MYSQL_BIND * deleteBindMem;
                int * deleteDataLength;

                Resource() {
                    initialized = false;
                    deleteInitialized = false;
                }

                ~Resource() {
                    if (initialized) {
                        mysql_stmt_close(resource->insertStatement);
                        delete[] resource->insertMem;
                        delete[] resource->insertBindMem;
                        delete[] resource->insertDataLength;
                        
                        mysql_stmt_close(resource->updateStatement);
                        delete[] resource->updateMem;
                        delete[] resource->updateBindMem;
                        delete[] resource->updateDataLength;
                    }
                    
                    if (deleteInitialized) {
                        mysql_stmt_close(resource->deleteStatement);
                        delete[] resource->deleteMem;
                        delete[] resource->deleteBindMem;
                        delete[] resource->deleteDataLength;
                    }
                }
        };

        static boost::thread_specific_ptr<Resource> resource;
        
        template<class __Item>
        class GetFieldNamesProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    std::vector<std::string> * fieldNames = \
                            (std::vector<std::string> *)param_ptr;
                    fieldNames->push_back(__Item::stringify());
                }
        };
        
        class InsertBindsPrepareProcedureParam
        {
            public:
                int offset;
                int index;
                Resource * resource;
        };
        template<class __Item>
        class InsertBindsPrepareProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    InsertBindsPrepareProcedureParam * param = \
                            (InsertBindsPrepareProcedureParam *)param_ptr;
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(param->resource->insertMem + param->offset, value->get_max_length());
                    param->resource->insertDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->insertBindMem + param->index,
                            value.get(),
                            param->resource->insertDataLength + param->index, true);
                
                    param->index -= 1;
                    param->offset += __Item::max_length;
                }
        };
        
        class InsertParamCopyProcedureParam
        {
            public:
                int offset;
                int index;
                Subclass * model;
                Resource * resource;
        };
        template<class __Item>
        class InsertParamCopyProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    typedef typename __Item::itemType ValueType;
                    typedef typename ValueType::cpptype cpptype;

                    InsertParamCopyProcedureParam * param = \
                            (InsertParamCopyProcedureParam *)param_ptr;
                    
                    cpptype cppvalue = __Item::getCppValue(param->model);
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_cpptype(cppvalue));
                    value->bind(param->resource->insertMem + param->offset, value->get_max_length());
                    param->resource->insertDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->insertBindMem + param->index,
                            value.get(),
                            param->resource->insertDataLength + param->index, true);
                
                    param->index -= 1;
                    param->offset += ValueType::max_length;
                }
        };
        
        class UpdateBindsPrepareProcedureParam
        {
            public:
                int offset;
                int index;
                Resource * resource;
        };
        template<class __Item>
        class UpdateBindsPrepareProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    UpdateBindsPrepareProcedureParam * param = \
                            (UpdateBindsPrepareProcedureParam *)param_ptr;
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(param->resource->updateMem + param->offset, value->get_max_length());
                    param->resource->updateDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->updateBindMem + param->index,
                            value.get(),
                            param->resource->updateDataLength + param->index, true);
                
                    param->index -= 1;
                    param->offset += __Item::max_length;
                }
        };
        
        class UpdateParamCopyProcedureParam
        {
            public:
                int offset;
                int index;
                Subclass * model;
                Resource * resource;
        };
        template<class __Item>
        class UpdateParamCopyProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    typedef typename __Item::itemType ValueType;
                    typedef typename ValueType::cpptype cpptype;

                    UpdateParamCopyProcedureParam * param = \
                            (UpdateParamCopyProcedureParam *)param_ptr;
                    
                    cpptype cppvalue = __Item::getCppValue(param->model);
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_cpptype(cppvalue));
                    value->bind(param->resource->updateMem + param->offset, value->get_max_length());
                    param->resource->updateDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->updateBindMem + param->index,
                            value.get(),
                            param->resource->updateDataLength + param->index, true);
                
                    param->index -= 1;
                    param->offset += ValueType::max_length;
                }
        };
        
        static void prepare(MYSQL_BIND * bind, const AbstractValue * value, int * length, bool is_input,
                my_bool * is_null = NULL,
                my_bool is_unsigned = 0,
                my_bool * error = NULL) {
            std::memset(bind, 0, sizeof(bind));
            if (is_input) {
                bind->buffer_type = (enum_field_types)value->get_mysql_in_type_code();
            } else {
                bind->buffer_type = (enum_field_types)value->get_mysql_out_type_code();
            }
            bind->buffer = value->get_data_address();
            bind->buffer_length = value->get_max_data_length();
            bind->length = (unsigned long *)length;
            bind->is_null = is_null;
            bind->is_unsigned = is_unsigned;
            bind->error = error;
        }
};

template<class Subclass>
boost::thread_specific_ptr< typename Model<Subclass>::Resource > Model<Subclass>::resource;

#define BEGIN_MODEL(name) \
    class name: public Model<name>, SqlQueryPart \
    { \
        public: \
            typedef name cls;\
            static std::string stringify() { return # name ; }\
            virtual ~name() {}\
            \
            IntValue::cpptype id;\
            class _id_: public SqlQueryPart \
            { \
                public: \
                    static std::string stringify() { return "id"; } \
                    typedef IntValue itemType; \
                    static typename itemType::cpptype getCppValue(cls * instance) { \
                        return instance->id; \
                    } \
                    static void setCValue(cls * instance, typename itemType::ctype value) { \
                        instance->id = (typename itemType::cpptype)value; \
                    } \
                    static void setCppValue(cls * instance, typename itemType::cpptype value) { \
                        instance->id = value; \
                    } \
            }; \
            typedef TypePair< TypePair< TypePair<_id_, IntValue>, TypeNil>,

#define FIELD(name, ValueType) \
                ValueType> name ## List; \
    ValueType::cpptype name; \
    class _ ## name ## _: public SqlQueryPart \
    { \
        public: \
            static std::string stringify() { return # name ; } \
            typedef ValueType itemType; \
            static typename itemType::cpptype getCppValue(cls * instance) { \
                return instance->name; \
            } \
            static void setCValue(cls * instance, typename itemType::ctype value) {\
                instance->name = (typename itemType::cpptype)value;\
            }\
            static void setCppValue(cls * instance, typename itemType::cpptype value) {\
                instance->name = value;\
            }\
    };\
    typedef TypePair< TypePair< TypePair<_ ## name ## _, name ## List::tail>, name ## List::head>,

#define END_MODEL \
        TypeNil> wrongTypeList;\
        \
        typedef wrongTypeList::head typeZippedList;\
        typedef typename UnzipFirst<typeZippedList>::typeList fieldList;\
        typedef typename UnzipSecond<typeZippedList>::typeList typeList;\
    };

#endif //COT_MODEL_H
