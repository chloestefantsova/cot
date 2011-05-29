/**
 *
 *
 */ 


#ifndef TEMPLATE_UTILS
#define TEMPLATE_UTILS

#include <sstream>

/*
 * Macros for defining static string containing structs.
 */
#define NAME_VALUE(T, name, value) \
		struct name \
		{ \
			static T v() \
			{ \
				return value;\
			} \
		};
#define STR(T, value) \
	NAME_VALUE(T, value, #value)

/*
 * NIL struct. Use for end of list.
 */
struct nil
{
	static std::string v(const char* delimiter = "")
	{
		return "";
	}
	static void v_tail(std::stringstream& ss, const char* delimiter = "")
	{
	}
};

/**
 * Meta pair struct. Use to make meta list.
 */
template<typename first, typename second>
struct pair
{
	typedef first head;
	typedef second tail;
	/**
	 * Method to print meta list to std::string
	 */
	static std::string v(const char* delimiter = "")
	{
		std::stringstream ss;
		ss << head::v();
		tail::v_tail(ss, delimiter);
		return ss.str();
	}
	static void v_tail(std::stringstream& ss, const char* delimiter = "")
	{
		ss << delimiter << head::v();
	  	tail::v_tail(ss, delimiter);
	}
};

#endif // TEMPLATE_UTILS
