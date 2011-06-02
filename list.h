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

#endif //COT_LIST_H
