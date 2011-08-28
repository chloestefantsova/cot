#ifndef COT_MODEL_H
#define COT_MODEL_H

#include <string>
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
                // FIXME: add initialization code here
            }
        }
        
    private:
    
        static bool initialized;
};

#define BEGIN_MODEL(name) \
    class name: public Model<name>, SqlQueryPart \
    { \
        public: \
            typedef name cls;\
            static std::string stringify() { return # name ; }\
            \
            IntValue::cpptype id;\
            class _id_: public SqlQueryPart \
            { \
                public: \
                    static std::string stringify() { return "id"; } \
                    typedef IntValue itemType; \
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
