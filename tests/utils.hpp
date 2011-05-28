#ifndef TEMPLATE_UTILS
#define TEMPLATE_UTILS

/*
 * Macros for defining static string containing structs.
 */
#define STR(value) \
	namespace strings \
	{ \
		struct value \
		{ \
			static std::string v() \
			{\
				return # value; \
			}\
		};\
	}
#define STR_VALUE(str) \
	strings::str::v()

/*
 * NIL struct. Use for end of list.
 */
struct nil{};

/**
 * Meta pair struct. Use to make meta list.
 */
template<typename first, typename second>
struct pair
{
	typedef first head;
	typedef second tail;
};


/**
 * Meta functor for concatting string lists.
 * ToDo: upgrade to std::sstream making it to concat any types.
 */
template<typename list>
struct concat_list
{
	static std::string v(const char* delimiter = " ")
	{
		return list::head::v() + concat_list<typename list::tail>::v_tail(delimiter);
	}

	static std::string v_tail(const char* delimiter = " ")
	{
		return std::string(delimiter) + list::head::v() + concat_list<typename list::tail>::v_tail(delimiter);
	}
};
template<>
struct concat_list<nil>
{
	static std::string v(const char* delimiter = "")
	{
		return "";
	}

	static std::string v_tail(const char* delimiter = "")
	{
		return "";
	}
};

#endif // TEMPLATE_UTILS
