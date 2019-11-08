#include "ParserTests.hpp"

#include "../../Parser/Parser.hpp"

namespace Tests{
	using namespace Parser;
	namespace{
		void assertPageTokenize(std::string source, std::vector<Token> tokens){
			TokenedPage page = tokenizePage(source);
			assertEqualsVec(tokens, page.tokens);
		}
	}
	
	void addParserTests(Tester& tester){
		tester.add("Parser::normalizePageName", [](){
			assertEquals("testing-page-name", normalizePageName("testing-page-name"));
			assertEquals("testing-page2-name", normalizePageName("TeStInG, page2;? \"name\""));
			assertEquals("cat:page", normalizePageName("cat:page"));
			assertEquals("cat:page", normalizePageName(" - cat - : - page - "));
			assertEquals("what:c-at:pa-ge", normalizePageName("--what--: - c at - : - pa ge - "));
			assertEquals("page#toc", normalizePageName("page#toc"));
		});
		
		tester.add("Parser::getPageLinks", [](){
			assertEqualsVec({
				"page-a",
				"page-b",
				"page-c"
			}, getPageLinks("[https://google.com] https://google.com [[[https://google.com]]] [[[page-a]]] [[[page \"\"b;|link name]]] [[[*pAge\nC]]] [!--[[[page-d]]]--]"));
		});
		
		tester.add("Parser::tokenizePage Basic", [](){
			assertPageTokenize("", {});
			assertPageTokenize("help", {
				Token{PlainText{"help"}, 0, 4, "help"}
			});
			assertPageTokenize("help\n\nme", {
				Token{PlainText{"help"}, 0, 4, "help"},
				Token{NewLine{}, 4, 5, "\n"},
				Token{NewLine{}, 5, 6, "\n"},
				Token{PlainText{"me"}, 6, 8, "me"}
			});
			assertPageTokenize("escaped\\\nnewline", {
				Token{PlainText{"escaped"}, 0, 7, "escaped"},
				Token{PlainText{"newline"}, 9, 16, "newline"}
			});
			assertPageTokenize("force _\nlinebreak", {
				Token{PlainText{"force"}, 0, 5, "force"},
				Token{LineBreak{}, 5, 8, " _\n"},
				Token{PlainText{"linebreak"}, 8, 17, "linebreak"}
			});
		});
		
		tester.add("Parser::tokenizePage Comment", [](){
			assertPageTokenize("[!-- comment --]", {
				
			});
			assertPageTokenize("text[!-- --]here", {
				Token{PlainText{"text"}, 0, 4, "text"},
				Token{PlainText{"here"}, 12, 16, "here"}
			});
			assertPageTokenize("A[!--B@@--]C@@D--]E", {
				Token{PlainText{"A"}, 0, 1, "A"},
				Token{PlainText{"E"}, 18, 19, "E"}
			});
			assertPageTokenize("A[!--B@@--]\n\nC@@D--]E", {
				Token{PlainText{"A"}, 0, 1, "A"},
				Token{NewLine{}, 11, 12, "\n"},
				Token{NewLine{}, 12, 13, "\n"},
				Token{PlainText{"C@@D—]E"}, 13, 21, "C@@D--]E"}
			});
		});
		
		tester.add("Parser::tokenizePage LiteralText", [](){
            assertPageTokenize("test@@[!--B--]@@comment", {
                Token{PlainText{"test"}, 0, 4, "test"},
                Token{LiteralText{"[!--B--]"}, 4, 16, "@@[!--B--]@@"},
				Token{PlainText{"comment"}, 16, 23, "comment"}
            });
            assertPageTokenize("test@@&lt;@@comment", {
                Token{PlainText{"test"}, 0, 4, "test"},
                Token{LiteralText{"&lt;"}, 4, 12, "@@&lt;@@"},
				Token{PlainText{"comment"}, 12, 19, "comment"}
            });
            assertPageTokenize("test@@\nliteral@@", {
                Token{PlainText{"test@@"}, 0, 6, "test@@"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{PlainText{"literal@@"}, 7, 16, "literal@@"}
            });
		});
		
		tester.add("Parser::tokenizePage EntityEscape", [](){
			assertPageTokenize("A@<[!--C--]>@B", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{LiteralText{"[!--C--]"}, 1, 13, "@<[!--C--]>@"},
				Token{PlainText{"B"}, 13, 14, "B"}
            });
            assertPageTokenize("A@<&lt;a&gt;>@B", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{LiteralText{"<a>"}, 1, 14, "@<&lt;a&gt;>@"},
				Token{PlainText{"B"}, 14, 15, "B"}
            });
			assertPageTokenize("A@<B\nC>@D", {
                Token{PlainText{"A@<B"}, 0, 4, "A@<B"},
                Token{NewLine{}, 4, 5, "\n"},
                Token{PlainText{"C>@D"}, 5, 9, "C>@D"}
            });
		});
		
		tester.add("Parser::tokenizePage Typography", [](){
			assertPageTokenize("`A'``B'',,C''D--E<<F>>G...", {
                Token{PlainText{"‘A’“B”„C”D—E«F»G…"}, 0, 26, "`A'``B'',,C''D--E<<F>>G..."}
            });
            assertPageTokenize("`A``B,,CD--E<<FG...\n\nA'B''C''D--EF>>G...", {
                Token{PlainText{"`A``B,,CD—E«FG…"}, 0, 19, "`A``B,,CD--E<<FG..."},
                Token{NewLine{}, 19, 20, "\n"},
                Token{NewLine{}, 20, 21, "\n"},
                Token{PlainText{"A'B''C''D—EF»G…"}, 21, 40, "A'B''C''D--EF>>G..."}
            });
		});
		
		tester.add("Parser::tokenizePage BareLink", [](){
			assertPageTokenize("https://google.com link", {
                Token{HyperLink{"https://google.com", "https://google.com", false}, 0, 18, "https://google.com"},
                Token{PlainText{" link"}, 18, 23, " link"}
            });
            assertPageTokenize("http://google.com link", {
                Token{HyperLink{"http://google.com", "http://google.com", false}, 0, 17, "http://google.com"},
                Token{PlainText{" link"}, 17, 22, " link"}
            });
            assertPageTokenize("*https://google.com link", {
                Token{HyperLink{"https://google.com", "https://google.com", true}, 0, 19, "*https://google.com"},
                Token{PlainText{" link"}, 19, 24, " link"}
            });
            assertPageTokenize("*http://google.com link", {
                Token{HyperLink{"http://google.com", "http://google.com", true}, 0, 18, "*http://google.com"},
                Token{PlainText{" link"}, 18, 23, " link"}
            });
		});
		
		tester.add("Parser::tokenizePage SingleLink", [](){
			assertPageTokenize("[https://google.com link here]", {
                Token{HyperLink{"link here", "https://google.com", false}, 0, 30, "[https://google.com link here]"}
            });
            assertPageTokenize("[http://google.com link here]", {
                Token{HyperLink{"link here", "http://google.com", false}, 0, 29, "[http://google.com link here]"}
            });
            assertPageTokenize("[*https://google.com link here]", {
                Token{HyperLink{"link here", "https://google.com", true}, 0, 31, "[*https://google.com link here]"}
            });
            assertPageTokenize("[*http://google.com link here]", {
                Token{HyperLink{"link here", "http://google.com", true}, 0, 30, "[*http://google.com link here]"}
            });
            
            ///TODO: these don't actually work in wikidot, singleLinks must have a specified shownName
            assertPageTokenize("[https://google.com]", {
                Token{HyperLink{"https://google.com", "https://google.com", false}, 0, 20, "[https://google.com]"}
            });
            assertPageTokenize("[http://google.com]", {
                Token{HyperLink{"http://google.com", "http://google.com", false}, 0, 19, "[http://google.com]"}
            });
            assertPageTokenize("[*https://google.com]", {
                Token{HyperLink{"https://google.com", "https://google.com", true}, 0, 21, "[*https://google.com]"}
            });
            assertPageTokenize("[*http://google.com]", {
                Token{HyperLink{"http://google.com", "http://google.com", true}, 0, 20, "[*http://google.com]"}
            });
		});
		
		tester.add("Parser::tokenizePage TripleLink", [](){
			assertPageTokenize("[[[test-page]]]", {
                Token{HyperLink{"test-page", "test-page", false}, 0, 15, "[[[test-page]]]"}
            });
            assertPageTokenize("[[[test-page|link]]]", {
                Token{HyperLink{"link", "test-page", false}, 0, 20, "[[[test-page|link]]]"}
            });
            assertPageTokenize("[[[TeStInG, page2;? \"name\"]]]", {
                Token{HyperLink{"TeStInG, page2;? \"name\"", "testing-page2-name", false}, 0, 29, "[[[TeStInG, page2;? \"name\"]]]"}
            });
            assertPageTokenize("[[[TeStInG, page2;? \"name\"|]]]", {
                Token{HyperLink{"testing-page2-name", "testing-page2-name", false}, 0, 30, "[[[TeStInG, page2;? \"name\"|]]]"}
            });
            assertPageTokenize("[[[*test-page]]]", {
                Token{HyperLink{"*test-page", "test-page", false}, 0, 16, "[[[*test-page]]]"}
            });
            assertPageTokenize("[[[*TeStInG, page2;? \"name\"]]]", {
                Token{HyperLink{"*TeStInG, page2;? \"name\"", "testing-page2-name", false}, 0, 30, "[[[*TeStInG, page2;? \"name\"]]]"}
            });
            assertPageTokenize("[[[test-page | shown name]]]", {
                Token{HyperLink{"shown name", "test-page", false}, 0, 28, "[[[test-page | shown name]]]"}
            });
            assertPageTokenize("[[[TeStInG, page2;? \"name\" | shown name]]]", {
                Token{HyperLink{"shown name", "testing-page2-name", false}, 0, 42, "[[[TeStInG, page2;? \"name\" | shown name]]]"}
            });
            assertPageTokenize("[[[# | java script]]]", {
                Token{HyperLink{"java script", "#", false}, 0, 21, "[[[# | java script]]]"}
            });
            
            assertPageTokenize("[[[https://google.com | google]]]", {
                Token{HyperLink{"google", "https://google.com", false}, 0, 33, "[[[https://google.com | google]]]"}
            });
            assertPageTokenize("[[[http://google.com | google]]]", {
                Token{HyperLink{"google", "http://google.com", false}, 0, 32, "[[[http://google.com | google]]]"}
            });
            assertPageTokenize("[[[*https://google.com | google]]]", {
                Token{HyperLink{"google", "https://google.com", true}, 0, 34, "[[[*https://google.com | google]]]"}
            });
            assertPageTokenize("[[[*http://google.com | google]]]", {
                Token{HyperLink{"google", "http://google.com", true}, 0, 33, "[[[*http://google.com | google]]]"}
            });
            
			assertPageTokenize("[[[https://google.com]]]", {
                Token{HyperLink{"https://google.com", "https://google.com", false}, 0, 24, "[[[https://google.com]]]"}
            });
            assertPageTokenize("[[[http://google.com]]]", {
                Token{HyperLink{"http://google.com", "http://google.com", false}, 0, 23, "[[[http://google.com]]]"}
            });
            assertPageTokenize("[[[*https://google.com]]]", {
                Token{HyperLink{"https://google.com", "https://google.com", true}, 0, 25, "[[[*https://google.com]]]"}
            });
            assertPageTokenize("[[[*http://google.com]]]", {
                Token{HyperLink{"http://google.com", "http://google.com", true}, 0, 24, "[[[*http://google.com]]]"}
            });
		});
		
		tester.add("Parser::tokenizePage Heading", [](){
			assertPageTokenize("+++ Heading", {
                Token{Heading{3, false}, 0, 4, "+++ "},
                Token{PlainText{"Heading"}, 4, 11, "Heading"}
            });
            assertPageTokenize("++* Heading", {
                Token{Heading{2, true}, 0, 4, "++* "},
                Token{PlainText{"Heading"}, 4, 11, "Heading"}
            });
            assertPageTokenize("+ Hea+ ding+", {
                Token{Heading{1, false}, 0, 2, "+ "},
                Token{PlainText{"Hea+ ding+"}, 2, 12, "Hea+ ding+"}
            });
        });
	}
}
