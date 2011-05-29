#include <string>
#include <iostream>

#include "utils.hpp"
#include "sql.hpp"

using std::string;

STR(string, SELECT);
STR(string, AUTHOR);
STR(string, FROM);
STR(string, AUTHORS);
STR(string, IZD);
NAME_VALUE(int, FIVE, 5)

typedef pair<SELECT, pair<AUTHOR, pair<FIVE, pair<FROM, pair<AUTHORS, nil> > > > > query;

typedef pair<AUTHOR, pair<IZD, nil> > attrib;

int main()
{
	std::cout << select< attrib, AUTHORS>::v() << std::endl;

	return 0;
}
