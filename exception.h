#ifndef __exception_h
#define __exception_h

#include <stdexcept>
#include <string>

class CotException: public std::runtime_error
{
    public:
        CotException();
        CotException(const std::string &);
        CotException(const CotException &);
        virtual ~CotException() throw();
};

#endif