#ifndef COT_CQL_H
#define COT_SQL_H

#include <cstdarg>
#include <cstring>
#include <stdint.h>
#include <boost/scoped_ptr.hpp>
#include <mysql/mysql.h>
#include "exception.h"
#include "type.h"
#include "list.h"

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

template<class __Model, class __Where>
class Select: public SqlQueryPart
{
    private:
    
        typedef typename __Where::typeList paramTypeList;
        typedef typename __Model::typeList resultTypeList;
    
        static uint8_t * paramMem;
        static const int paramMemSize = MemCount<paramTypeList>::value;
        
        static uint8_t * resultMem;
        static const int resultMemSize = __Model::totalFieldSize;
        
        static MYSQL_BIND * paramBindMem;
        static const int paramCount = Length<paramTypeList>::value;
        
        static MYSQL_BIND * resultBindMem;
        static const int resultCount = __Model::fieldCount;
        
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
        
        static int * paramDataLength;
        static int paramOffset;
        static int paramIndex;
        
        template<class __Item>
        class ParamPrepareProcedure
        {
            public:
                static void exec() {
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(paramMem + paramOffset, value->get_max_length());
                    paramDataLength[paramIndex] = value->get_data_length();
                    prepare(paramBindMem + paramIndex, value.get(), paramDataLength + paramIndex, true);
                    
                    paramOffset += value->get_max_length();
                    paramIndex += 1;
                }
        };
        
        static int * resultDataLength;
        static int resultOffset;
        static int resultIndex;
        
        template<class __Item>
        class ResultPrepareProcedure
        {
            public:
                static void exec() {
                    boost::scoped_ptr<AbstractValue> value(new __Item);
                    
                    value->bind(resultMem + resultOffset, value->get_max_length());
                    resultDataLength[resultIndex] = value->get_max_data_length();
                    prepare(resultBindMem + resultIndex, value.get(), resultDataLength + resultIndex, false);
                    
                    resultOffset += value->get_max_length();
                    resultIndex -= 1;
                }
        };
        
        static va_list ap;
        
        template<class __Item>
        class ParamCopyProcedure
        {
            public:
                static void exec() {
                    typedef typename __Item::ctype ctype;
                    
                    ctype cvalue = va_arg(ap,  ctype);
                    boost::scoped_ptr<AbstractValue> value(__Item::from_ctype(cvalue));
                    
                    value->bind(paramMem + paramOffset, value->get_max_length());
                    paramDataLength[paramIndex] = value->get_data_length();
                    prepare(paramBindMem + paramIndex, value.get(), paramDataLength + paramIndex, true);
                    
                    paramOffset += __Item::max_length;
                    paramIndex += 1;
                }
        };
        
        static __Model * currentModel;
        
        template<class __Item>
        class ResultCopyProcedure
        {
            public:
                static void exec() {
                    typedef typename __Item::itemType ValueType;
                    
                    boost::scoped_ptr<AbstractValue> value(ValueType::from_dump(resultMem + resultOffset));
                    __Item::setCppValue(currentModel, GET_CPP_REPR(ValueType, value));
                    
                    resultOffset += ValueType::max_length;
                    resultIndex -= 1;
                }
        };
        
        static MYSQL_STMT * statement;
    
    public:

        // It seems that there is no solution to the problem of
        // removing the first parameter of the variadic function.

        static std::vector<__Model *> with(int n, ...) {
            va_start(ap, n);
            
            if (!initialized) {
                // 0. Preparing statement
                statement = mysql_stmt_init(Connection::connection());
                
                std::string theQuery = stringify();
                char * theQueryCstr = new char[theQuery.length() + 1];
                std::strncpy(theQueryCstr, theQuery.c_str(), theQuery.length());
                if (mysql_stmt_prepare(statement, theQueryCstr, theQuery.length()) != 0) {
                    throw new CotException(std::string("mysql_stmt_prepare(): ") + mysql_stmt_error(statement));
                }
                delete[] theQueryCstr;
                
                // 1. Parameters
                paramMem = new uint8_t[paramMemSize]; // throws an exception if fails, ok for us
                paramBindMem = new MYSQL_BIND[paramCount]; // throws an exception if fails, ok for us
                paramDataLength = new int[paramCount]; // throws an exception if fails, ok for us
                
                paramOffset = 0;
                paramIndex = 0;
                Exec<paramTypeList, ParamPrepareProcedure>::exec();
                
                if (mysql_stmt_bind_param(statement, paramBindMem) != 0) {
                    throw new CotException(std::string("mysql_stmt_bind_param(): ") + \
                        mysql_stmt_error(statement));
                }

                // 2. Result
                resultMem = new uint8_t[resultMemSize]; // throws an exception if fails, ok for us
                resultBindMem = new MYSQL_BIND[resultCount]; // throws an exception if fails, ok for us
                resultDataLength = new int[resultCount]; // throws an exception if fails, ok for us
                
                resultOffset = 0;
                resultIndex = resultCount - 1;
                Exec<resultTypeList, ResultPrepareProcedure>::exec();
                
                if (mysql_stmt_bind_result(statement, resultBindMem) != 0) {
                    throw new CotException(std::string("mysql_stmt_bind_result(): ") + \
                        mysql_stmt_error(statement));
                }

                initialized = true;
            }
            
            // copy the given variadic parameter list into the param bind memory
            paramOffset = 0;
            paramIndex = 0;
            Exec<paramTypeList, ParamCopyProcedure>::exec();
            
            // execute the query
            if (mysql_stmt_execute(statement) != 0) {
                throw new CotException(std::string("mysql_stmt_execute(): ") + mysql_stmt_error(statement));
            }
            
            // fetching the result
            std::vector<__Model *> result;
            int retcode;
            while ((retcode = mysql_stmt_fetch(statement)) == 0 || retcode == MYSQL_DATA_TRUNCATED) {
                currentModel = new __Model;
                resultOffset = 0;
                resultIndex = resultCount - 1;
                Exec<typename __Model::fieldList, ResultCopyProcedure>::exec();
                result.push_back(currentModel);
            }
            if (retcode != MYSQL_NO_DATA) {
                throw new CotException(std::string("mysql_stmt_fetch(): ") + \
                    mysql_stmt_error(statement));
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
        
        static void destroy() {
            mysql_stmt_close(statement);
            delete[] paramMem;
            delete[] resultMem;
            delete[] paramBindMem;
            delete[] resultBindMem;
            delete[] paramDataLength;
            delete[] resultDataLength;
        }
        
    private:
        static bool initialized;
};

template<class __Model, class __Where>
uint8_t * Select<__Model, __Where>::paramMem = NULL;

template<class __Model, class __Where>
uint8_t * Select<__Model, __Where>::resultMem = NULL;

template<class __Model, class __Where>
MYSQL_BIND * Select<__Model, __Where>::paramBindMem = NULL;

template<class __Model, class __Where>
MYSQL_BIND * Select<__Model, __Where>::resultBindMem = NULL;

template<class __Model, class __Where>
int * Select<__Model, __Where>::paramDataLength = NULL;

template<class __Model, class __Where>
int Select<__Model, __Where>::paramOffset = 0;

template<class __Model, class __Where>
int Select<__Model, __Where>::paramIndex = 0;

template<class __Model, class __Where>
int * Select<__Model, __Where>::resultDataLength = NULL;

template<class __Model, class __Where>
int Select<__Model, __Where>::resultOffset = 0;

template<class __Model, class __Where>
int Select<__Model, __Where>::resultIndex = 0;

template<class __Model, class __Where>
bool Select<__Model, __Where>::initialized = false;

template<class __Model, class __Where>
va_list Select<__Model, __Where>::ap;

template<class __Model, class __Where>
__Model * Select<__Model, __Where>::currentModel = NULL;

template<class __Model, class __Where>
MYSQL_STMT * Select<__Model, __Where>::statement = NULL;

#endif // COT_SQL_H
