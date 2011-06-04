#ifndef __author_h
#define __author_h

#include <string>
#include <vector>
#include <cstdarg>
#include <iostream>        // see DEBUG
#include "model.h"

BEGIN_MODEL(Author)
    FIELD(name, StringValue<256>)
    FIELD(age, IntValue)
    FIELD(bookCount, IntValue)
END_MODEL

#endif
