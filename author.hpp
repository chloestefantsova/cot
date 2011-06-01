#ifndef __author_hpp
#define __author_hpp

#include <string>
#include <vector>
#include <cstdarg>
#include <iostream>        // see DEBUG
#include "model.h"

BEGIN_MODEL(Author)
    FIELD(std::string, name, StringValue<256>)
    FIELD(int, age, IntValue)
END_MODEL

#endif
