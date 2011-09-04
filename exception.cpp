#include "cot/exception.h"

namespace cot {

Exception::Exception():
    std::runtime_error("Exception")
{
}

Exception::Exception(const std::string & msg):
    std::runtime_error(std::string("Exception: ") + msg)
{
}

Exception::Exception(const Exception & ex):
    std::runtime_error(ex)
{
}

Exception::~Exception() throw()
{
}

} // namespace cot
