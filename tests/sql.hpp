#ifndef SQL_TEMPLATES
#define SQL_TEMPLATES

#include <string>
#include <sstream>
#include "utils.hpp"

#include <iostream>

template<typename attr, typename table, typename where = nil>
struct select
{
	static std::string v()
	{
		std::cerr << "select 3";
		std::stringstream ss;
		ss << "SELECT " << attr::v(", ") << " FROM " << table::v() << " WHERE " << where::v() << ";";
		return ss.str();
	}
};

template<typename attr, typename table>
struct select<attr, table, nil>
{
	static std::string v()
	{
//		std::cerr << "select2";
		std::stringstream ss;
		ss << "SELECT " << attr::v(", ") << " FROM " << table::v() << ";";
		return ss.str();
	}
};


#endif // SQL_TEMPLATES
