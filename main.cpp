#include <iostream>
#include <vector>
#include "connection.h"
#include "exception.h"
#include "author.h"

using namespace std;

void perform()
{
    Connection::connect("localhost", "cot_user", "cot_pass", "cot_db");

    // There are several things to note:

    // 1. Static variable _age_ is the metadatum describing the instance
    // variable age of class Author. Lowdashes (_) attached, for there
    // could be no instance variables and static variables with the same
    // name. (NOTE: same applies to _name_ variable)

    // 2.  It seems that the C++ 98 standard prohibits the usage of
    // values as template parameters for non-integer types.  Thus, all
    // query parameters should be passed as arguments to a variadic
    // member function.  The latter should guess the parameters types
    // from the compile-time list of parameter types and sizes.

    // 3.  As it reveals, the variadic functions should have at least
    // one argument with a name.  Since there is no way to omit the
    // latter, it would be used as argument counter.

    vector<Author *> authors = 
        Select< Author, Where<
            Or<
                Lt<Author::_age_>,
                Eq<Author::_name_> > > >::with(2, 41, "John Smith");

    for (int i = 0; i < (int)authors.size(); ++i) {
        cout << authors[i]->id << " | " \
                << authors[i]->name << " | " \
                << authors[i]->age << " | " \
                << authors[i]->bookCount << endl;
    }
    
    Connection::disconnect();
}

int main(int argc, char * argv[])
{
    try {
        perform();
    } catch (const std::runtime_error * error) {
        cerr << error->what() << endl;
    }
    
    return 0;
}
