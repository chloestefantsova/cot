#ifndef __connection_h
#define __connection_h

#include <map>
#include <vector>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <mysql/mysql.h>

// Connection class is a singleton.  There would be the connections
// pool in the upcomming release.

class Connection
{
    private:
        typedef void (*destroy_pointer_t)();

        class Resource
        {
            public:
                MYSQL connect;

                ~Resource() {
                    mysql_close(&connect);
                }
        };

        static boost::thread_specific_ptr<Resource> resource;

    public:
        static void connect(const char *,   /* host */
                            const char *,   /* user */
                            const char *,   /* password */
                            const char *);  /* db name */

        static MYSQL * connection();
};

#endif
