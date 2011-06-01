#ifndef COT_MODEL_H
#define COT_MODEL_H

#include "sql.h"
#include "type.h"

#define BEGIN_MODEL(name) \
    class name: public SqlQueryPart \
    { \
        public: \
            static std::string stringify() { return # name ; }\
        typedef TypePair<TypeNil,

#define FIELD(cType, name, iType) \
                iType> name ## List; \
    cType name; \
    class _ ## name ## _: public SqlQueryPart \
    { \
        public: \
            static std::string stringify() { return # name ; } \
            typedef iType itemType; \
    };\
    typedef TypePair< name ## List,

#define END_MODEL \
        TypeNil> wrongTypeList;\
        /*typedef Reverse<wrongTypeList::head>::list typeList;*/\
    };

#endif //COT_MODEL_H
