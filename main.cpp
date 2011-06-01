#include <iostream>
#include <vector>
#include "author.hpp"

using namespace std;

int main(int argc, char * argv[])
{
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
            And<
                Or<Lt<Author::_age_>,Gt<Author::_age_,Author::_age_> >,
                Eq<Author::_name_> > > >::with(2, 40, "John Smith");

    for (int i = 0; i < (int)authors.size(); ++i) {
        cout << authors[i]->name << " of age " << authors[i]->age << endl;
    }

    return 0;
}
