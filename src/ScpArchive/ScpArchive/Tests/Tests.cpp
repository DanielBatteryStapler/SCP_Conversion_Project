#include "AutomatedTester.hpp"

#include "Database/DatabaseTests.hpp"
#include "Database/JsonTests.hpp"

namespace Tests{
	void runAllTests(){
		Tester tester;
		
		addJsonTests(tester);
		addDatabaseTests(tester);
		
		addTesterTests(tester);
		
		tester.runTests();
	}
}
