/*
 * select * from Author;
 */

#include <iostream>
#include <vector>
#include "author.hpp"

using namespace std;

int main()
{
    Connection::connect("localhost",
                        "orm_user",
                        "orm_pass",
                        "orm_db");

    vector<Author *> result = Author::all();

    for (int i = 0; i < (int)result.size(); ++i) {
        cout << result[i]->name
             << " is of age "
             << result[i]->age
             << endl;
    }

    Connection::disconnect();

	return 0;
}
