#ifndef __connection_h
#define __connection_h

#include <mysql/mysql.h>

// Connection class is a singleton.  There would be the connections
// pool in the upcomming release.

class Connection
{
    public:
        static void connect(const char *,   /* host */
                            const char *,   /* user */
                            const char *,   /* password */
                            const char *);  /* db name */

        static void disconnect();

        static MYSQL * connection();

    private:
        static MYSQL _connect;
};

#endif
