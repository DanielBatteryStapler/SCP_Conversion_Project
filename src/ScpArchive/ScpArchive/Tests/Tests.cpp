#include "AutomatedTester.hpp"

#include "Database/ImporterTests.hpp"
#include "Database/DatabaseTests.hpp"
#include "Database/JsonTests.hpp"

#include "Parser/MarkupOutStreamTests.hpp"

namespace Tests{
	void runAllTests(){
		Tester tester;
		
		addMarkupOutStreamTests(tester);
		
		addImporterTests(tester);
		addJsonTests(tester);
		addDatabaseTests(tester);
		
		addTesterTests(tester);
		
		tester.runTests();
	}
}
