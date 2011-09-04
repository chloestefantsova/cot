#include "cot/exception.h"

CotException::CotException():
    std::runtime_error("CotException")
{
}

CotException::CotException(const std::string & msg):
    std::runtime_error(std::string("CotException: ") + msg)
{
}

CotException::CotException(const CotException & ex):
    std::runtime_error(ex)
{
}

CotException::~CotException() throw()
{
}
