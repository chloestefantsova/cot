#ifndef COT_TYPE_H
#define COT_TYPE_H

// Classes containing the information about field types are held
// withing this file.  The type information includes:

// - memType - the type to which void * pointing to intermediate
// buffer should be casted in order to get the base field value;

// - castType - the type to which the base field value should be
// casted in order to obtain the thread safe copy of the base field value
// and reuse the intermediate buffer later;

// - size - the size of intermediate buffer area (in bytes) that holds
// both the base field value and auxiliary data for it.

// The resulting routine to get the actual field data is:
// castType(memType(void * pointing to the intermediate buffer area))

#include <string>

template<class __MemType, class __CastType, int __size>
class Value
{
    public:
        typedef __MemType memType;
        typedef __CastType castType;
        static const int size = __size;
        
        static __CastType get(void * addr) {
            return __CastType(__MemType(addr));
        }
};

class IntValue: public Value<int, int, sizeof(int)> {};

// The auxiliary data includes null byte for C-string and the pointer
// to the begining of the actual string (in fact, the next byte after the
// pointer).

template<int length>
class StringValue: public Value<char *, std::string, sizeof(char *) + length + 1> {};

#endif // COT_TYPE_H

