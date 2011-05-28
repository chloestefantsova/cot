#ifndef __author_hpp
#define __author_hpp

#include <vector>
#include <mysql/mysql.h>

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

class Author
{
    public:
        int age;
        char * name;

        Author();
        ~Author();
        void save();

        static std::vector<Author *> all();

    private:
        static const char query_all[];
        static MYSQL_STMT * query_all_stmt; // where to close?
};

#endif
