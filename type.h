#ifndef COT_TYPE_H
#define COT_TYPE_H

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


#endif // COT_TYPE_H

