#ifndef COT_CQL_H
#define COT_SQL_H

#include <cstdarg>
#include <cstring>
#include <stdint.h>
#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <mysql/mysql.h>
#include "cot/exception.h"
#include "cot/type.h"
#include "cot/list.h"

class Parameter
{
    public:
	    static std::string stringify() { return "?"; }
};

class SqlQueryPart
{
    public:
        static std::string stringify() { return ""; }
};

template<class __MetaField1, class __MetaField2 = Parameter>
class SqlCmp: public SqlQueryPart
{
    public:
        static std::string stringify() { return ""; }

        typedef TypeNil typeList;
};

template<class __MetaField1>
class SqlCmp<__MetaField1, Parameter>: public SqlQueryPart
{
    public:
        static std::string stringify() { return ""; }

        typedef TypePair<typename __MetaField1::itemType, TypeNil> typeList;
};


template<class __MetaField1, class __MetaField2 = Parameter>
class Lt: public SqlCmp<__MetaField1, __MetaField2>
{
    public:
        static std::string stringify() {
            return __MetaField1::stringify() + " < " + __MetaField2::stringify();
        }
};

template<class __MetaField1, class __MetaField2 = Parameter>
class Eq: public SqlCmp<__MetaField1, __MetaField2>
{
    public:
        static std::string stringify() {
            return __MetaField1::stringify() + " = " + __MetaField2::stringify();
        }
};

template<class __MetaField1, class __MetaField2 = Parameter>
class Gt: public SqlCmp<__MetaField1, __MetaField2>
{
    public:
        static std::string stringify() {
            return __MetaField1::stringify() + " > " + __MetaField2::stringify();
        }
};

template<class __First, class __Second>
class SqlBinOp: public SqlQueryPart
{
    public:
        static std::string stringify() { return ""; }
        typedef
            typename Append<typename __First::typeList, typename __Second::typeList>::typeList
                    typeList;
};

template<class __First, class __Second>
class And: public SqlBinOp<__First, __Second>
{
    public:
        static std::string stringify() {
            return std::string("(") + __First::stringify() + " and " + __Second::stringify() + ")";
        }
};

template<class __First, class __Second>
class Or: public SqlBinOp<__First, __Second>
{
    public:
        static std::string stringify() {
            return std::string("(") + __First::stringify() + " or " + __Second::stringify() + ")";
        }
};

template<class __BooleanExp>
class Where: public SqlQueryPart
{
    public:
        static std::string stringify() {
            return std::string("where ") + __BooleanExp::stringify();
        }
        typedef typename __BooleanExp::typeList typeList;
};

class All: public SqlQueryPart
{
    public:
        static std::string stringify() { return ""; }
        typedef TypeNil typeList;
};

template<class __Model, class __Where>
class Select: public SqlQueryPart
{
    private:
    
        typedef typename __Where::typeList paramTypeList;
        typedef typename __Model::typeList resultTypeList;
    
        static const int paramMemSize = MemCount<paramTypeList>::value;
        static const int resultMemSize = __Model::totalFieldSize;
        static const int paramCount = Length<paramTypeList>::value;
        static const int resultCount = __Model::fieldCount;

        class Resource
        {
            public:
                bool initialized;

                uint8_t * paramMem;
                MYSQL_BIND * paramBindMem;
                int * paramDataLength;

                uint8_t * resultMem;
                MYSQL_BIND * resultBindMem;
                int * resultDataLength;

                MYSQL_STMT * statement;

                Resource() {
                    initialized = false;
                }

                ~Resource() {
                    mysql_stmt_close(statement);
                    delete[] paramMem;
                    delete[] resultMem;
                    delete[] paramBindMem;
                    delete[] resultBindMem;
                    delete[] paramDataLength;
                    delete[] resultDataLength;
                }
        };

        static boost::thread_specific_ptr<Resource> resource;
        
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
        
        class ParamPrepareProcedureParam
        {
            public:
                int offset;
                int index;
                Resource * resource;
        };
        template<class __Item>
        class ParamPrepareProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    ParamPrepareProcedureParam * param = (ParamPrepareProcedureParam *)param_ptr;
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(param->resource->paramMem + param->offset, value->get_max_length());
                    param->resource->paramDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->paramBindMem + param->index,
                            value.get(),
                            param->resource->paramDataLength + param->index,
                            true);
                    
                    param->offset += value->get_max_length();
                    param->index += 1;
                }
        };
        
        class ResultPrepareProcedureParam
        {
            public:
                int offset;
                int index;
                Resource * resource;
        };
        template<class __Item>
        class ResultPrepareProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    ResultPrepareProcedureParam * param = (ResultPrepareProcedureParam *)param_ptr;
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(param->resource->resultMem + param->offset, value->get_max_length());
                    param->resource->resultDataLength[param->index] = value->get_max_data_length();
                    prepare(param->resource->resultBindMem + param->index,
                            value.get(),
                            param->resource->resultDataLength + param->index,
                            false);
                    
                    param->offset += value->get_max_length();
                    param->index -= 1;
                }
        };
        
        class ParamCopyProcedureParam
        {
            public:
                int offset;
                int index;
                va_list * ap;
                Resource * resource;
        };
        template<class __Item>
        class ParamCopyProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    ParamCopyProcedureParam * param = (ParamCopyProcedureParam *)param_ptr;
                    typedef typename __Item::ctype ctype;
                    
                    ctype cvalue = va_arg(*param->ap,  ctype);
                    boost::scoped_ptr<AbstractValue> value(__Item::from_ctype(cvalue));
                    
                    value->bind(param->resource->paramMem + param->offset, value->get_max_length());
                    param->resource->paramDataLength[param->index] = value->get_data_length();
                    prepare(param->resource->paramBindMem + param->index,
                            value.get(),
                            param->resource->paramDataLength + param->index,
                            true);
                    
                    param->offset += __Item::max_length;
                    param->index += 1;
                }
        };
        
        class ResultCopyProcedureParam
        {
            public:
                int offset;
                int index;
                __Model * model;
                Resource * resource;
        };
        template<class __Item>
        class ResultCopyProcedure
        {
            public:
                static void exec(void * param_ptr) {
                    ResultCopyProcedureParam * param = (ResultCopyProcedureParam *)param_ptr;
                    typedef typename __Item::itemType ValueType;
                    
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_dump(param->resource->resultMem + param->offset));
                    __Item::setCppValue(param->model, GET_CPP_REPR(ValueType, value));
                    
                    param->offset += ValueType::max_length;
                    param->index -= 1;
                }
        };
    
    public:

        // It seems that there is no solution to the problem of
        // removing the first parameter of the variadic function.

        static std::vector< boost::shared_ptr<__Model> > with(int n, ...) {
            va_list ap;
            va_start(ap, n);

            if (resource.get() == NULL || !resource->initialized) {
                resource.reset(new Resource());

                // 0. Preparing statement
                resource->statement = mysql_stmt_init(Connection::connection());
                
                std::string theQuery = stringify();
                char * theQueryCstr = new char[theQuery.length() + 1];
                std::strncpy(theQueryCstr, theQuery.c_str(), theQuery.length());
                if (mysql_stmt_prepare(resource->statement, theQueryCstr, theQuery.length()) != 0) {
                    delete[] theQueryCstr;
                    throw new CotException(std::string("mysql_stmt_prepare(): ") + \
                            mysql_stmt_error(resource->statement));
                }
                delete[] theQueryCstr;
                
                // 1. Parameters
                resource->paramMem = new uint8_t[paramMemSize]; // throws an exception if fails, ok for us
                resource->paramBindMem = new MYSQL_BIND[paramCount]; // throws an exception if fails, ok for us
                resource->paramDataLength = new int[paramCount]; // throws an exception if fails, ok for us
                
                ParamPrepareProcedureParam ppp_param;
                ppp_param.offset = 0;
                ppp_param.index = 0;
                ppp_param.resource = resource.get();
                Exec<paramTypeList, ParamPrepareProcedure>::exec(&ppp_param);
                
                if (mysql_stmt_bind_param(resource->statement, resource->paramBindMem) != 0) {
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                        mysql_stmt_error(resource->statement));
                }

                // 2. Result
                resource->resultMem = new uint8_t[resultMemSize]; // throws an exception if fails, ok for us
                resource->resultBindMem = new MYSQL_BIND[resultCount]; // throws an exception if fails, ok for us
                resource->resultDataLength = new int[resultCount]; // throws an exception if fails, ok for us
                
                ResultPrepareProcedureParam rpp_param;
                rpp_param.offset = 0;
                rpp_param.index = resultCount - 1;
                rpp_param.resource = resource.get();
                Exec<resultTypeList, ResultPrepareProcedure>::exec(&rpp_param);
                
                if (mysql_stmt_bind_result(resource->statement, resource->resultBindMem) != 0) {
                    throw new CotException(std::string("mysql_stmt_bind_result(): ") + \
                        mysql_stmt_error(resource->statement));
                }

                resource->initialized = true;
            }
            
            // copy the given variadic parameter list into the param bind memory
            ParamCopyProcedureParam pcp_param;
            pcp_param.offset = 0;
            pcp_param.index = 0;
            pcp_param.ap = &ap;
            pcp_param.resource = resource.get();
            Exec<paramTypeList, ParamCopyProcedure>::exec(&pcp_param);
            
            // execute the query
            if (mysql_stmt_execute(resource->statement) != 0) {
                throw new CotException(std::string("mysql_stmt_execute(): ") + \
                        mysql_stmt_error(resource->statement));
            }
            
            // fetching the result
            std::vector< boost::shared_ptr<__Model> > result;
            int retcode;
            while ((retcode = mysql_stmt_fetch(resource->statement)) == 0 || retcode == MYSQL_DATA_TRUNCATED) {
                ResultCopyProcedureParam rcp_param;
                rcp_param.model = new __Model;
                rcp_param.offset = 0;
                rcp_param.index = resultCount - 1;
                rcp_param.resource = resource.get();
                Exec<typename __Model::fieldList, ResultCopyProcedure>::exec(&rcp_param);
                result.push_back(boost::shared_ptr<__Model>(rcp_param.model));
            }
            if (retcode != MYSQL_NO_DATA) {
                throw new CotException(std::string("mysql_stmt_fetch(): ") + \
                    mysql_stmt_error(resource->statement));
            }

            va_end(ap);
            return result;
        }

        static std::string stringify() {
            return std::string("select * from ") + \
                __Model::stringify() + \
                " " + \
                __Where::stringify();
        }
};

template<class __Model, class __Where>
boost::thread_specific_ptr< typename Select<__Model, __Where>::Resource >
Select<__Model, __Where>::resource;

#endif // COT_SQL_H
