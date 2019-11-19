#include "AutomatedTester.hpp"

#include "Database/ImporterTests.hpp"
#include "Database/DatabaseTests.hpp"
#include "Database/JsonTests.hpp"

#include "HTTP/HtmlEntityTests.hpp"
#include "HTTP/MarkupOutStreamTests.hpp"

#include "Parser/ParserTests.hpp"
#include "Parser/TreerTests.hpp"

namespace Tests{
	void runAllTests(){
		Tester tester;
		
		addParserTests(tester);
		addTreerTests(tester);
		
		addHtmlEntityTests(tester);
		addMarkupOutStreamTests(tester);
		
		addImporterTests(tester);
		addJsonTests(tester);
		addDatabaseTests(tester);
		
		addTesterTests(tester);
		
		tester.runTests();
	}
}
