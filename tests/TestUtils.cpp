/*
 * hamming.cpp
 *
 *  Created on: 20.03.2011
 *      Author: raya
 */

#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "utils.hpp"
using std::string;
STR(string, SELECT)
STR(string, AUTHOR)
STR(string, FROM)
STR(string, AUTHORS)
NAME_VALUE(int, FIVE, 5)

typedef pair<SELECT, pair<AUTHOR, pair<FIVE, pair<FROM, pair<AUTHORS, nil> > > > > query;



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
protected:

		CPPUNIT_TEST_SUITE( TestUtils );
	CPPUNIT_TEST( testStrPair );
	CPPUNIT_TEST( testStr );
	CPPUNIT_TEST( testPairList );
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

