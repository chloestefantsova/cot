#ifndef __author_hpp
#define __author_hpp

#include <string>
#include <vector>
#include <cstdarg>
#include <iostream>	// see DEBUG

class IntValue
{
    public:
        typedef int type;
        static const int size = sizeof(int);
};

template<int length>
class StringValue
{
    public:
        typedef char * type;
        static const int size = sizeof(char *) + length + 1; // pointer plus string plus a null byte
};

class TypeNil {};

template<class First, class Second>
class TypePair
{
    public:
        typedef First head;
        typedef Second tail;
};

template<class __List, class __Item>
class AppendItem
{
    public:
        typedef TypePair<
            typename __List::head,
            typename AppendItem<typename __List::tail, __Item>::typeList>
        typeList;
};

template<class __Item>
class AppendItem<TypeNil, __Item>
{
    public:
        typedef TypePair<__Item, TypeNil> typeList;
};

template<class __ListA, class __ListB>
class Append
{
    public:
        typedef typename Append<
            typename AppendItem<__ListA, typename __ListB::head>::typeList,
            typename __ListB::tail >::typeList
        typeList;
};

template<class __List>
class Append<__List, TypeNil>
{
    public:
        typedef __List typeList;
};

template<class __List>
class Length
{
    public:
        static const int value = Length<typename __List::tail>::value + 1;
};

template<>
class Length<TypeNil>
{
    public:
        static const int value = 0;
};

template< class __List, template<class __Item> class __Procedure >
class Exec
{
    public:
        static void exec() {
            __Procedure<typename __List::head>::exec();
            Exec<typename __List::tail, __Procedure>::exec();
        }
};

template< template<class __Item> class __Procedure >
class Exec<TypeNil, __Procedure>
{
    public:
        static void exec() {}
};

class SqlQueryPart
{
    public:
        SqlQueryPart() {}
        virtual ~SqlQueryPart() {}
        static std::string stringify() { return ""; }
};

template<class __MetaField>
class SqlCmp: public SqlQueryPart
{
    public:
        SqlCmp () {}
        virtual ~SqlCmp() {}
        static std::string stringify() { return ""; }
        typedef TypePair<typename __MetaField::itemType, TypeNil> typeList;
};

template<class __MetaField>
class Lt: public SqlCmp<__MetaField>
{
    public:
        Lt() {}
        virtual ~Lt() {}
        static std::string stringify() {
            return __MetaField::stringify() + " < ?";
        }
};

template<class __MetaField>
class Eq: public SqlCmp<__MetaField>
{
    public:
        Eq() {}
        virtual ~Eq() {}
        static std::string stringify() {
            return __MetaField::stringify() + " = ?";
        }
};

template<class __MetaField>
class Gt: public SqlCmp<__MetaField>
{
    public:
        Gt() {}
        virtual ~Gt() {}
        static std::string stringify() {
            return __MetaField::stringify() + " > ?";
        }
};

template<class __First, class __Second>
class SqlBinOp: public SqlQueryPart
{
    public:
        SqlBinOp() {}
        virtual ~SqlBinOp() {}
        static std::string stringify() { return ""; }
        typedef
            typename Append<typename __First::typeList, typename __Second::typeList>::typeList
        typeList;
};

template<class __First, class __Second>
class And: public SqlBinOp<__First, __Second>
{
    public:
        And() {}
        virtual ~And() {}
        static std::string stringify() {
            return std::string("(") + __First::stringify() + " and " + __Second::stringify() + ")";
        }
};

template<class __BooleanExp>
class Where: public SqlQueryPart
{
    public:
        Where() {}
        virtual ~Where() {}
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
        Select() {}
        virtual ~Select() {}

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

#define BEGIN_MODEL(name) \
    class name: public SqlQueryPart \
    { \
        public: \
            name() {} \
            virtual ~name() {} \
            static std::string stringify() { return # name ; }

#define FIELD(cType, name, iType) \
    cType name; \
    class _ ## name ## _: public SqlQueryPart \
    { \
        public: \
            _ ## name ## _ () {} \
            virtual ~_ ## name ## _() {} \
            static std::string stringify() { return # name ; } \
            typedef iType itemType; \
    };

#define END_MODEL \
    };

BEGIN_MODEL(Author)
    FIELD(std::string, name, StringValue<256>)
    FIELD(int, age, IntValue)
END_MODEL

#endif
