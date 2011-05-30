#ifndef __author_hpp
#define __author_hpp

#include <string>
#include <vector>
#include <cstdarg>

class TypeNil {};

template<class First, class Second>
class TypePair
{
    public:
        typedef First Head;
        typedef Second Tail;
};

class SqlQueryPart
{
    public:
        SqlQueryPart() {}
        virtual ~SqlQueryPart() {}
        static std::string stringify() { return ""; }
};

template<class __MetaField>
class SqlBinOp: public SqlQueryPart
{
    public:
        SqlBinOp () {}
        virtual ~SqlBinOp() {}
        static std::string stringify() { return ""; }
};

template<class __MetaField>
class Lt: public SqlBinOp<__MetaField>
{
    public:
        Lt() {}
        virtual ~Lt() {}
        static std::string stringify() {
            return __MetaField::stringify() + " < ?";
        }
};

template<class __MetaField>
class Eq: public SqlBinOp<__MetaField>
{
    public:
        Eq() {}
        virtual ~Eq() {}
        static std::string stringify() {
            return __MetaField::stringify() + " = ?";
        }
};

template<class __MetaField>
class Gt: public SqlBinOp<__MetaField>
{
    public:
        Gt() {}
        virtual ~Gt() {}
        static std::string stringify() {
            return __MetaField::stringify() + " > ?";
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
};

template<class __Model, class __Where>
class Select: public SqlQueryPart
{
    public:
        Select() {}
        virtual ~Select() {}

	// It seems that there is no solution to the problem of
	// removing the first parameter of the variadic function.

        static std::vector<__Model *> with(int n, ...) {
            va_list ap;
            va_start(ap, n);

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
};

#define BEGIN_MODEL(name) \
    class name: public SqlQueryPart \
    { \
        public: \
            name() {} \
            virtual ~name() {} \
            static std::string stringify() { return # name ; }

#define FIELD(type, name) \
    type name; \
    class _ ## name ## _: public SqlQueryPart \
    { \
        public: \
            _ ## name ## _ () {} \
            virtual ~_ ## name ## _() {} \
            static std::string stringify() { return # name ; } \
    };

#define END_MODEL \
    };

BEGIN_MODEL(Author)
    FIELD(std::string, name)
    FIELD(int, age)
END_MODEL

#endif
