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
	}
}
