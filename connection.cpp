#include <vector>
#include <string>
#include <boost/thread/tss.hpp>
#include "cot/connection.h"
#include "cot/exception.h"

namespace cot {

boost::thread_specific_ptr<Connection::Resource> Connection::resource;

void Connection::connect(
    const char * host,
    const char * user,
    const char * password,
    const char * name)
{
    if (resource.get() == NULL) {
        resource.reset(new Resource());
    }

    if (!mysql_init(&resource->connect))
        throw Exception(
            std::string("mysql_init(): ") +
            mysql_error(&resource->connect)
        );

    if (!mysql_real_connect(
            &resource->connect,
            host, user, password, name,
            0,
            NULL,
            0))
        throw Exception(
            std::string("mysql_real_connect()") +
            mysql_error(&resource->connect)
        );
}

MYSQL * Connection::connection()
{
    return &resource->connect;
}

} // namespace cot
