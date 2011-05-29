#include <cstring>
#include "author.hpp"

const std::string Author::query_all = select< pair<AGE,pair<NAME,nil>>, TABLENAME>::v();

Author::Author()
{
}

Author::~Author()
{
}

void Author::save()
{
}

std::vector<Author *> Author::all()
{
}
