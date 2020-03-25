#include "HTMLConverterTests.hpp"

#include "../../Parser/HTMLConverter.hpp"

namespace Tests{
	using namespace Parser;
	
	void addHTMLConverterTests(Tester& tester){
		tester.add("Parser::redirectLink", [](){
			assertEquals("/scp-173", redirectLink("http://www.scp-wiki.net/scp-173"));
			assertEquals("/scp-173", redirectLink("http://scp-wiki.wikidot.com/scp-173"));
			assertEquals("/__system/pageFile/scp-173/SCP-173.jpg", redirectLink("http://scp-wiki.wdfiles.com/local--files/scp-173/SCP-173.jpg"));
			assertEquals("/__system/pageFile/page/image.jpg", redirectLink("http://www.scp-wiki.net/local--files/page/image.jpg"));
			assertEquals("/scp-173", redirectLink("/scp-173"));
			assertEquals("https://google.com/", redirectLink("https://google.com/"));
		});
	}
}
