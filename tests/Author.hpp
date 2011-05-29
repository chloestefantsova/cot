#define TABLE(name)\
	NAME_VALUE(string, TABLE, name)

class Author
{
	TABLE(AUTHORS);
	public:
		int age;
		COLUMN(age, AUTHORS_AGE, INTEGER);
		char * name;
		COLUMN(name, AUTHORS_NAME, VARCHAR);
		
		Author();
		~Author();
		void save();
		
		static std::vector<Author *> all();
		
	private:
		static const char query_all[];
		static MYSQL_STMT * query_all_stmt; // where to close?
};
