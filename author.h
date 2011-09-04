#ifndef __author_h
#define __author_h

#include <cstdarg>
#include "cot/model.h"

BEGIN_MODEL(Author)
    FIELD(name, cot::StringValue<256>)
    FIELD(age, cot::IntValue)
    FIELD(bookCount, cot::IntValue)
END_MODEL

#endif
