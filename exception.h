#ifndef __exception_h
#define __exception_h

#include <stdexcept>
#include <string>

namespace cot {

class Exception: public std::runtime_error
{
    public:
        Exception();
        Exception(const std::string &);
        Exception(const Exception &);
        virtual ~Exception() throw();
};

} // namespace cot

#endif
