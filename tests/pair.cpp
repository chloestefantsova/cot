#include <string>
#include <iostream>

#include "utils.hpp"


STR(SELECT);
STR(AUTHOR);
STR(FROM);
STR(AUTHORS);

using namespace strings;

typedef pair<SELECT, pair<AUTHOR, pair<FROM, pair<AUTHORS, nil> > > > query;

int main()
{
	std::cout << pair<SELECT, int>::head::v() ;
	std::cout << STR_VALUE(SELECT) << std::endl;
	std::cout << concat_list<query>::v(", ") << std::endl;

	return 0;
}
