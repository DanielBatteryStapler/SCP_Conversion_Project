#include "TemplaterTests.hpp"

#include "../../Parser/Templater.hpp"

namespace Tests{
	using namespace Parser;
	
	void addTemplaterTests(Tester& tester){
		tester.add("Parser::applyIncludeParameters", [](){
			assertEquals(
                "this was replaced"
                , applyIncludeParameters("this {$varA} replaced", {{"varA", "was"}}));
		});
		
		tester.add("Parser::applyPageTemplate", [](){
			assertEquals(
                "just ignored"
                , applyPageTemplate("just ignored", "page content"));
                
			assertEquals(
                "page content"
                , applyPageTemplate("%%content%%", "page content"));
                
            assertEquals(
                "page:\n"
                "page content"
                , applyPageTemplate("page:\n%%content%%", "page content"));
			
			//due to performance issues the following has been disabled
			/*
            assertEquals(
                "page A:\n"
                "page content A\n"
                "page B:\n"
                "page content B"
                , applyPageTemplate("page A:\n%%content(1)%%page B:%%content(2)%%", "page content A\n=======\npage content B"));
			*/
		});
	}
}
