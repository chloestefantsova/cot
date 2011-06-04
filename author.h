#ifndef __author_h
#define __author_h

#include <cstdarg>
#include "model.h"

BEGIN_MODEL(Author)
    FIELD(name, StringValue<256>)
    FIELD(age, IntValue)
    FIELD(bookCount, IntValue)
END_MODEL

#endif
