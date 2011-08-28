#ifndef COT_MODEL_H
#define COT_MODEL_H

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "list.h"
#include "type.h"

template<class Subclass>
class Model
{
    public:
        static const int fieldCount = Length<typename Subclass::fieldList>::value;
        static const int totalFieldSize = MemCount<typename Subclass::typeList>::value;
        
        virtual ~Model() {}
        
        void save() {
            if (!initialized) {
            
                // 1. INSERT
                
                // 1.1. Get the INSERT query string.
                insertQueryStr = std::string("INSERT INTO ") + Subclass::stringify() + " VALUES ";
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
                insertStatement = mysql_stmt_init(Connection::connection());
                char * insertQueryCStr = new char[insertQueryStr.length() + 1];
                std::strncpy(insertQueryCStr, insertQueryStr.c_str(), insertQueryStr.length());
                if (mysql_stmt_prepare(insertStatement, insertQueryCStr, insertQueryStr.length()) != 0) {
                    delete[] insertQueryCStr;
                    throw new CotException(std::string("mysql_stmt_prepare(): ") +\
                            mysql_stmt_error(insertStatement));
                }
                delete[] insertQueryCStr;
                
                // 1.3. Prepare MYSQL_BIND structures for the INSERT query.
                insertMem = new uint8_t[totalFieldSize];
                insertBindMem = new MYSQL_BIND[fieldCount];
                insertDataLength = new int[fieldCount];
                insertIndex = fieldCount - 1;
                insertOffset = 0;
                Exec<typename Subclass::typeList, InsertBindsPrepareProcedure>::exec();
                if (mysql_stmt_bind_param(insertStatement, insertBindMem + 1) != 0) { // except the id
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                            mysql_stmt_error(insertStatement));
                }
                
                // 2. UPDATE
                
                // 2.1. Get the UPDATE query string.
                Exec<typename Subclass::fieldList, GetFieldNamesProcedure>::exec();
                updateQueryStr = std::string("UPDATE ") + Subclass::stringify() + " SET ";
                for (int i = fieldCount - 2; i >= 0; --i) { // except the id, in normal order
                    updateQueryStr += fieldNames[i] + " = ?";
                    if (i != 0) { updateQueryStr += ", "; }
                }
                updateQueryStr += " WHERE id = ?";
                
                // 2.2. Prepare the UPDATE statement
                updateStatement = mysql_stmt_init(Connection::connection());
                char * updateQueryCStr = new char[updateQueryStr.length() + 1];
                std::strncpy(updateQueryCStr, updateQueryStr.c_str(), updateQueryStr.length());
                if (mysql_stmt_prepare(updateStatement, updateQueryCStr, updateQueryStr.length()) != 0) {
                    delete[] updateQueryCStr;
                    throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                            mysql_stmt_error(updateStatement));
                }
                delete[] updateQueryCStr;
                
                // 2.3. Prepare MYSQL_BIND structures for the UPDATE query.
                updateMem = new uint8_t[totalFieldSize];
                updateBindMem = new MYSQL_BIND[fieldCount + 1];
                updateDataLength = new int[fieldCount];
                updateIndex = fieldCount - 1;
                updateOffset = 0;
                Exec<typename Subclass::typeList, UpdateBindsPrepareProcedure>::exec();
                std::memcpy(updateBindMem + fieldCount, updateBindMem, sizeof(MYSQL_BIND));
                if (mysql_stmt_bind_param(updateStatement, updateBindMem + 1) != 0) { // moved id
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                            mysql_stmt_error(updateStatement));
                }
                
                initialized = true;
            }
            
            if (Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this)) == 0) {
                // INSERT
                insertIndex = fieldCount - 1;
                insertOffset = 0;
                currentModel = reinterpret_cast<Subclass *>(this);
                Exec<typename Subclass::fieldList, InsertParamCopyProcedure>::exec();
                
                if (mysql_stmt_execute(insertStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(insertStatement));
                }
                
                Subclass::_id_::setCppValue(
                        reinterpret_cast<Subclass *>(this),
                        (int)mysql_insert_id(Connection::connection()));
            } else {
                // UPDATE
                updateIndex = fieldCount - 1;
                updateOffset = 0;
                currentModel = reinterpret_cast<Subclass *>(this);
                Exec<typename Subclass::fieldList, UpdateParamCopyProcedure>::exec();
                std::memcpy(updateBindMem + fieldCount, updateBindMem, sizeof(MYSQL_BIND));
                
                if (mysql_stmt_execute(updateStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(updateStatement));
                }
            }
        }
        
        void remove() {
            if (Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this)) != 0) {
                if (!deleteInitialized) {
                    // NOTE: the query string is always the same.
                    
                    // 1. Prepare the statement.
                    deleteStatement = mysql_stmt_init(Connection::connection());
                    std::string deleteQueryStr = \
                            std::string("DELETE FROM ") + Subclass::stringify() + " WHERE id = ?";
                    char * deleteQueryCStr = new char[deleteQueryStr.length() + 1];
                    std::strncpy(deleteQueryCStr, deleteQueryStr.c_str(), deleteQueryStr.length());
                    if (mysql_stmt_prepare(deleteStatement, deleteQueryCStr, deleteQueryStr.length()) != 0) {
                        delete[] deleteQueryCStr;
                        throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                                mysql_stmt_error(deleteStatement));
                    }
                    delete[] deleteQueryCStr;
                    
                    // 2. Bind the parameter.
                    deleteMem = new uint8_t[IntValue::max_length];
                    deleteBindMem = new MYSQL_BIND[1];
                    deleteDataLength = new int[1];
                    boost::scoped_ptr<AbstractValue> value(new IntValue);
                    value->bind(deleteMem, value->get_max_length());
                    deleteDataLength[0] = value->get_data_length();
                    prepare(deleteBindMem, value.get(), deleteDataLength, true);
                    if (mysql_stmt_bind_param(deleteStatement, deleteBindMem) != 0) {
                        throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                                mysql_stmt_error(deleteStatement));
                    }
                    
                    deleteInitialized = true;
                }
                
                IntValue::cpptype cppvalue = \
                        Subclass::_id_::getCppValue(reinterpret_cast<Subclass *>(this));
                boost::scoped_ptr<AbstractValue> value(IntValue::from_cpptype(cppvalue));
                value->bind(deleteMem, value->get_max_length());
                deleteDataLength[0] = value->get_data_length();
                prepare(deleteBindMem, value.get(), deleteDataLength, true);
                
                if (mysql_stmt_execute(deleteStatement) != 0) {
                    throw new CotException(std::string("mysql_stmt_execute(): ") + \
                            mysql_stmt_error(deleteStatement));
                }
                
                Subclass::_id_::setCppValue(reinterpret_cast<Subclass *>(this), 0);
            }
        }
        
        static void destroy() {
            mysql_stmt_close(insertStatement);
            delete[] insertMem;
            delete[] insertBindMem;
            delete[] insertDataLength;
            
            mysql_stmt_close(updateStatement);
            delete[] updateMem;
            delete[] updateBindMem;
            delete[] updateDataLength;
            
            mysql_stmt_close(deleteStatement);
        }
        
    protected:
    
        static std::string insertQueryStr;
        static std::string updateQueryStr;
        static bool initialized;
        static bool deleteInitialized;
        static Subclass * currentModel;
        
        static std::vector<std::string> fieldNames;
        
        template<class __Item>
        class GetFieldNamesProcedure
        {
            public:
                static void exec() {
                    fieldNames.push_back(__Item::stringify());
                }
        };
        
        static MYSQL_STMT * insertStatement;
        static uint8_t * insertMem;
        static MYSQL_BIND * insertBindMem;
        static int * insertDataLength;
        static int insertIndex;
        static int insertOffset;
        
        template<class __Item>
        class InsertBindsPrepareProcedure
        {
            public:
                static void exec() {
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(insertMem + insertOffset, value->get_max_length());
                    insertDataLength[insertIndex] = value->get_data_length();
                    prepare(insertBindMem + insertIndex, value.get(), insertDataLength + insertIndex, true);
                
                    insertIndex -= 1;
                    insertOffset += __Item::max_length;
                }
        };
        
        template<class __Item>
        class InsertParamCopyProcedure
        {
            public:
                static void exec() {
                    typedef typename __Item::itemType ValueType;
                    typedef typename ValueType::cpptype cpptype;
                    
                    cpptype cppvalue = __Item::getCppValue(currentModel);
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_cpptype(cppvalue));
                    value->bind(insertMem + insertOffset, value->get_max_length());
                    insertDataLength[insertIndex] = value->get_data_length();
                    prepare(insertBindMem + insertIndex, value.get(), insertDataLength + insertIndex, true);
                
                    insertIndex -= 1;
                    insertOffset += ValueType::max_length;
                }
        };
        
        static MYSQL_STMT * updateStatement;
        static uint8_t * updateMem;
        static MYSQL_BIND * updateBindMem;
        static int * updateDataLength;
        static int updateIndex;
        static int updateOffset;
        
        template<class __Item>
        class UpdateBindsPrepareProcedure
        {
            public:
                static void exec() {
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(updateMem + updateOffset, value->get_max_length());
                    updateDataLength[updateIndex] = value->get_data_length();
                    prepare(updateBindMem + updateIndex, value.get(), updateDataLength + updateIndex, true);
                
                    updateIndex -= 1;
                    updateOffset += __Item::max_length;
                }
        };
        
        template<class __Item>
        class UpdateParamCopyProcedure
        {
            public:
                static void exec() {
                    typedef typename __Item::itemType ValueType;
                    typedef typename ValueType::cpptype cpptype;
                    
                    cpptype cppvalue = __Item::getCppValue(currentModel);
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_cpptype(cppvalue));
                    value->bind(updateMem + updateOffset, value->get_max_length());
                    updateDataLength[updateIndex] = value->get_data_length();
                    prepare(updateBindMem + updateIndex, value.get(), updateDataLength + updateIndex, true);
                
                    updateIndex -= 1;
                    updateOffset += ValueType::max_length;
                }
        };
        
        static MYSQL_STMT * deleteStatement;
        static uint8_t * deleteMem;
        static MYSQL_BIND * deleteBindMem;
        static int * deleteDataLength;
        
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
bool Model<Subclass>::initialized = false;

template<class Subclass>
bool Model<Subclass>::deleteInitialized = false;

template<class Subclass>
std::string Model<Subclass>::insertQueryStr;

template<class Subclass>
std::string Model<Subclass>::updateQueryStr;

template<class Subclass>
Subclass * Model<Subclass>::currentModel = NULL;

template<class Subclass>
std::vector<std::string> Model<Subclass>::fieldNames;

template<class Subclass>
MYSQL_STMT * Model<Subclass>::insertStatement = NULL;

template<class Subclass>
uint8_t * Model<Subclass>::insertMem = NULL;

template<class Subclass>
MYSQL_BIND * Model<Subclass>::insertBindMem = NULL;

template<class Subclass>
int * Model<Subclass>::insertDataLength = NULL;

template<class Subclass>
int Model<Subclass>::insertIndex = 0;

template<class Subclass>
int Model<Subclass>::insertOffset = 0;

template<class Subclass>
MYSQL_STMT * Model<Subclass>::updateStatement = NULL;

template<class Subclass>
uint8_t * Model<Subclass>::updateMem = NULL;

template<class Subclass>
MYSQL_BIND * Model<Subclass>::updateBindMem = NULL;

template<class Subclass>
int * Model<Subclass>::updateDataLength = NULL;

template<class Subclass>
int Model<Subclass>::updateIndex = 0;

template<class Subclass>
int Model<Subclass>::updateOffset = 0;

template<class Subclass>
MYSQL_STMT * Model<Subclass>::deleteStatement = NULL;

template<class Subclass>
uint8_t * Model<Subclass>::deleteMem = NULL;

template<class Subclass>
MYSQL_BIND * Model<Subclass>::deleteBindMem = NULL;

template<class Subclass>
int * Model<Subclass>::deleteDataLength = NULL;

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
