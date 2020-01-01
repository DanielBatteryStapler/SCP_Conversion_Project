#include "AutomatedTester.hpp"

#include "Database/ImporterTests.hpp"
#include "Database/DatabaseTests.hpp"
#include "Database/JsonTests.hpp"

#include "HTTP/HtmlEntityTests.hpp"
#include "HTTP/MarkupOutStreamTests.hpp"

#include "Parser/ParserTests.hpp"
#include "Parser/TreerTests.hpp"
#include "Parser/TemplaterTests.hpp"

namespace Tests{
	void runAllTests(){
		Tester tester;
		
		addParserTests(tester);
		addTreerTests(tester);
		addTemplaterTests(tester);
		
		addHtmlEntityTests(tester);
		addMarkupOutStreamTests(tester);
		
		addImporterTests(tester);
		addJsonTests(tester);
		addDatabaseTests(tester);
		
		addTesterTests(tester);
		
		tester.runTests();
	}
}
