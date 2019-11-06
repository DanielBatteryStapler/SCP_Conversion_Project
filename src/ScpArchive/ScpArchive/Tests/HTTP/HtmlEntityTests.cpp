#include "HtmlEntityTests.hpp"

#include "../../HTTP/HtmlEntity.hpp"

namespace Tests{
	void addHtmlEntityTests(Tester& tester){
		tester.add("decodeHtmlEntities", [](){
			//this function was obtained from an outside source so we're just gonna hope that it works correctly
			//but we'll have a couple basic tests for good measure
			assertEquals("hello, world!", decodeHtmlEntities("hello, world!"));
			assertEquals("<a>", decodeHtmlEntities("&lt;a&gt;"));
		});
	}
}
