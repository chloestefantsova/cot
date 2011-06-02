#ifndef __author_hpp
#define __author_hpp

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
