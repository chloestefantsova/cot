#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
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


class TestUtils : public CppUnit::TestFixture
{
public:
	void testStrPair()
	{
		std::string str =  pair<SELECT, int>::head::v();
		CPPUNIT_ASSERT_EQUAL(string("SELECT"),str);
	}

	void testStr()
	{
		CPPUNIT_ASSERT_EQUAL(string("SELECT"), SELECT::v());
	}

	void testPairList()
	{
		CPPUNIT_ASSERT_EQUAL(string("SELECT, AUTHOR, 5, FROM, AUTHORS"), query::v(", "));
	}
	void testSelectBasic()
	{   
		string str = select< attrib, AUTHORS>::v();
		CPPUNIT_ASSERT_EQUAL(string("SELECT AUTHOR, IZD FROM AUTHORS;"), str);
	}  
protected:

	CPPUNIT_TEST_SUITE( TestUtils );
		CPPUNIT_TEST( testStrPair );
		CPPUNIT_TEST( testStr );
		CPPUNIT_TEST( testPairList );
		CPPUNIT_TEST( testSelectBasic );
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( TestUtils );

int main(int argc, char **argv)
{
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	registry.registerFactory(&CppUnit::TestFactoryRegistry::getRegistry("TestUtils"));
	runner.addTest(registry.makeTest());
	bool result;

	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cout));
	result = runner.run();

	return !result;
}

