#ifndef COT_LIST_H
#define COT_LIST_H

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
class UnzipFirst
{
    public:
        typedef TypePair<
            typename __List::head::head, 
            typename UnzipFirst<typename __List::tail>::typeList >
        typeList;
};
template<>
class UnzipFirst<TypeNil>
{
    public:
        typedef TypeNil typeList;
};

template<class __List>
class UnzipSecond
{
    public:
        typedef TypePair<
            typename __List::head::tail,
            typename UnzipSecond<typename __List::tail>::typeList >
        typeList;
};
template<>
class UnzipSecond<TypeNil>
{
    public:
        typedef TypeNil typeList;
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

        static void exec(void * param) {
            __Procedure<typename __List::head>::exec(param);
            Exec<typename __List::tail, __Procedure>::exec(param);
        }
};

template< template<class __Item> class __Procedure >
class Exec<TypeNil, __Procedure>
{
    public:
        static void exec() {}
        static void exec(void * param) {}
};

#define DECLARE_SUM_PROC(proc_name, field_name) \
        template<class TypeList> \
        class proc_name \
        { \
            public: \
                static const int value = TypeList::head::field_name + \
                        proc_name<typename TypeList::tail>::value; \
        }; \
        template<> \
        class proc_name<TypeNil> \
        { \
            public: \
                static const int value = 0; \
        };
        
DECLARE_SUM_PROC(MemCount, max_length)

#endif //COT_LIST_H
