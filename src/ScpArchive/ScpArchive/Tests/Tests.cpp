#include "AutomatedTester.hpp"

#include "Database/DatabaseTests.hpp"

namespace Tests{
	void runAllTests(){
		Tester tester;
		
		addDatabaseTests(tester);
		
		addTesterTests(tester);
		
		tester.runTests();
	}
}
