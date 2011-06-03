#include <string>
#include "connection.h"
#include "exception.h"

MYSQL Connection::_connect;

void Connection::connect(
    const char * host,
    const char * user,
    const char * password,
    const char * name)
{
    if (!mysql_init(&_connect))
        throw CotException(
            std::string("mysql_init(): ") +
            mysql_error(&_connect)
        );

    if (!mysql_real_connect(
            &_connect,
            host, user, password, name,
            0,
            NULL,
            0))
        throw CotException(
            std::string("mysql_real_connect()") +
            mysql_error(&_connect)
        );
}

void Connection::disconnect()
{
    mysql_close(&_connect);
}

MYSQL * Connection::connection()
{
    return &_connect;
}
