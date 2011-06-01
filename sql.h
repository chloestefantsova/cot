#ifndef COT_CQL_H
#define COT_SQL_H

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
        static int paramMemSize;
        
        template<class __Item>
        class ParamMemSizeProcedure
        {
            public:
                static void exec() {
                    paramMemSize += __Item::size;
                }
        };
    
    public:

        // It seems that there is no solution to the problem of
        // removing the first parameter of the variadic function.

        static std::vector<__Model *> with(int n, ...) {
            va_list ap;
            va_start(ap, n);
            
            if (!initialized) {
                // FIXME: write initialization instructions:
                // parameters / result bindings, memory allocation etc.
                
                // 1. Parameters
                typedef typename __Where::typeList paramTypeList;
                
                // DEBUG
                std::cout << "there are " << Length<paramTypeList>::value << " parameters" << std::endl;
                Exec<paramTypeList, ParamMemSizeProcedure>::exec();
                std::cout << "parameters need " << paramMemSize << " bytes in memory" << std::endl;
                paramMemSize = 0;
                //std::cout << "there are " << Length<typename __Model::typeList>::value << " results" << std::endl;
                //Exec<typename __Model::typeList, ParamMemSizeProcedure>::exec();
                //std::cout << "parameters need " << paramMemSize << " bytes in memory" << std::endl;


                // 2. Result
                // FIXME: result is to be done.

                initialized = true;
            }

            __Model * model = new __Model();
            model->name = stringify();
            model->age = 31337;

            va_end(ap);
            return std::vector<__Model *>(5, model);
        }

        static std::string stringify() {
            return std::string("select * from ") + \
                __Model::stringify() + \
                " " + \
                __Where::stringify();
        }
        
    private:
        static bool initialized;
};

template<class __Model, class __Where>
int Select<__Model, __Where>::paramMemSize = 0;

template<class __Model, class __Where>
bool Select<__Model, __Where>::initialized = false;


#endif // COT_SQL_H
