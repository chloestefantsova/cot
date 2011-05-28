#ifndef SQL_TEMPLATES
#define SQL_TEMPLATES

#include <string>
using std::string;



template< class attributes, char* table >
struct select
{
	static string get_query()
	{
		return "select " + concat_string_list<attributes>() + " from " + table;
	}

}


#endif // SQL_TEMPLATES
