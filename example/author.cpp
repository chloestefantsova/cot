#include <cstring>
#include "author.hpp"

MYSQL Connection::_connect;

void Connection::connect(const char * host,
                         const char * user,
                         const char * password,
                         const char * name)
{
    if (!mysql_init(&_connect))
        throw "mysql_init()";

    if (!mysql_real_connect(&_connect,
                            host, user, password, name,
                            0,
                            NULL,
                            0))
        throw "mysql_real_connect()";
}

void Connection::disconnect()
{
    mysql_close(&_connect);
}

MYSQL * Connection::connection()
{
    return &_connect;
}

const char Author::query_all[] = "select * from autor";
MYSQL_STMT * Author::query_all_stmt = NULL;

Author::Author()
{
    MYSQL * con = Connection::connection();

    if (query_all_stmt == NULL) {
        if ((query_all_stmt = mysql_stmt_init(con)) == NULL)
            throw "mysql_stmt_init()";
        if (mysql_stmt_prepare(query_all_stmt,
                               query_all,
                               std::strlen(query_all)) != 0)
            throw "mysql_stmt_prepare()";
    }
}

Author::~Author()
{
    delete[] name;  // ?
}

void Author::save()
{
}

std::vector<Author *> Author::all()
{
    if (mysql_stmt_execute(query_all_stmt) != 0)
        throw "mysql_stmt_execute";
    return std::vector<Author*>();
}
