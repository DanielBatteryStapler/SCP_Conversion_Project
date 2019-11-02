#include "MarkupOutStreamTests.hpp"

#include "../../Parser/MarkupOutStream.hpp"

#include <sstream>

namespace Tests{
	void addMarkupOutStreamTests(Tester& tester){
		tester.add("MarkupOutStream", [](){
			const auto escape = [](std::string testInput){
				std::stringstream output;
				MarkupOutStream stream(&output);
				stream << testInput;
				return output.str();
			};
			
			assertEquals("hello, test text", escape("hello, test text"));
			assertEquals("&lt;html&gt;is escaped&lt;/html&gt;", escape("<html>is escaped</html>"));
			assertEquals("&amp;amp&#59;", escape("&amp;"));
			assertEquals("&lt;a href=&#39;google.com&#39;&gt;link&lt;/a&gt;", escape("<a href='google.com'>link</a>"));
			assertEquals("&lt;a href=&quot;google.com&quot;&gt;link&lt;/a&gt;", escape("<a href=\"google.com\">link</a>"));
			
			std::stringstream output;
			MarkupOutStream stream(&output);
			stream << "<"_AM << "<" << allowMarkup(">");
			assertEquals("<&lt;>", output.str());
		});
		
		tester.add("percentEncode", [](){
			assertEquals("hello+world%21", urlEncode("hello world!"));
			assertEquals("", urlEncode(""));
			assertEquals("hello", urlEncode("hello"));
			assertEquals("hey%2C+what%27s+going+on%3F", urlEncode("hey, what's going on?"));
			assertEquals("this+is%2Fwas+madness", urlEncode("this is/was madness"));
			assertEquals("first%0D%0Asecond", urlEncode("first\r\nsecond"));
		});
		
		tester.add("percentDecode", [](){
			assertEquals("hello world!", urlDecode("hello+world%21"));
			assertEquals("", urlDecode(""));
			assertEquals("hello", urlDecode("hello"));
			assertEquals("hey, what's going on?", urlDecode("hey%2C+what%27s+going+on%3F"));
			assertEquals("this is/was madness", urlDecode("this+is%2Fwas+madness"));
			assertEquals("first\r\nsecond", urlDecode("first%0D%0Asecond"));
		});
	}
}
