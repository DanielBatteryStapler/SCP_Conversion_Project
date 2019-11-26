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
			assertPageTokenize("ignore\n  \n \n \n  \n pre-space", {
				Token{PlainText{"ignore"}, 0, 6, "ignore"},
				Token{NewLine{}, 6, 7, "\n"},
				Token{NewLine{}, 9, 10, "\n"},
				Token{NewLine{}, 11, 12, "\n"},
				Token{NewLine{}, 13, 14, "\n"},
				Token{NewLine{}, 16, 17, "\n"},
				Token{PlainText{"pre-space"}, 18, 27, "pre-space"}
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
				Token{PlainText{"C@@D"}, 13, 17, "C@@D"},
				Token{InlineFormat{InlineFormat::Type::Strike, true, true}, 17, 19, "--"},
				Token{PlainText{"]E"}, 19, 21, "]E"}
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
			assertPageTokenize("`A'``B'',,C''D -- E<<F>>G...", {
                Token{PlainText{"‘A’“B”„C”D — E«F»G…"}, 0, 28, "`A'``B'',,C''D -- E<<F>>G..."}
            });
            assertPageTokenize("`A``B,,CD -- E<<FG...\n\nA'B''C''D -- EF>>G...", {
                Token{PlainText{"`A``B"}, 0, 5, "`A``B"},
                Token{InlineFormat{InlineFormat::Type::Sub, true, true}, 5, 7, ",,"},
                Token{PlainText{"CD — E«FG…"}, 7, 21, "CD -- E<<FG..."},
                Token{NewLine{}, 21, 22, "\n"},
                Token{NewLine{}, 22, 23, "\n"},
                Token{PlainText{"A'B''C''D — EF»G…"}, 23, 44, "A'B''C''D -- EF>>G..."}
            });
		});
		
		tester.add("Parser::tokenizePage BareLink", [](){
			assertPageTokenize("https://google.com link", {
                Token{HyperLink{"https://google.com", "https://google.com", false}, 0, 18, "https://google.com"},
                Token{PlainText{" link"}, 18, 23, " link"}
            });
            assertPageTokenize("https://google.com[!--comment--]", {
                Token{HyperLink{"https://google.com", "https://google.com", false}, 0, 18, "https://google.com"}
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
            
            assertPageTokenize("[#toc go here]", {
                Token{HyperLink{"go here", "#toc", false}, 0, 14, "[#toc go here]"}
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
            assertPageTokenize("+ + Heading", {
                Token{Heading{1, false}, 0, 2, "+ "},
                Token{PlainText{"+ Heading"}, 2, 11, "+ Heading"}
            });
            assertPageTokenize("+ Hea+ ding+", {
                Token{Heading{1, false}, 0, 2, "+ "},
                Token{PlainText{"Hea+ ding+"}, 2, 12, "Hea+ ding+"}
            });
        });
        
        
        tester.add("Parser::tokenizePage QuoteBoxPrefix", [](){
            assertPageTokenize("> quote", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{PlainText{"quote"}, 2, 7, "quote"}
            });
            assertPageTokenize("> + quote", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{Heading{1, false}, 2, 4, "+ "},
                Token{PlainText{"quote"}, 4, 9, "quote"}
            });
            assertPageTokenize("> > quote", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{PlainText{"> quote"}, 2, 9, "> quote"}
            });
            assertPageTokenize(">> quote", {
                Token{QuoteBoxPrefix{2}, 0, 3, ">> "},
                Token{PlainText{"quote"}, 3, 8, "quote"}
            });
            assertPageTokenize("> quote\n>> quote #2", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{PlainText{"quote"}, 2, 7, "quote"},
                Token{NewLine{}, 7, 8, "\n"},
                Token{QuoteBoxPrefix{2}, 8, 11, ">> "},
                Token{PlainText{"quote #2"}, 11, 19, "quote #2"}
            });
            assertPageTokenize("> quote\n>> quote #2", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{PlainText{"quote"}, 2, 7, "quote"},
                Token{NewLine{}, 7, 8, "\n"},
                Token{QuoteBoxPrefix{2}, 8, 11, ">> "},
                Token{PlainText{"quote #2"}, 11, 19, "quote #2"}
            });
            assertPageTokenize("> quote\n>\n> line", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{PlainText{"quote"}, 2, 7, "quote"},
                Token{NewLine{}, 7, 8, "\n"},
                Token{QuoteBoxPrefix{1}, 8, 9, ">"},
                Token{NewLine{}, 9, 10, "\n"},
                Token{QuoteBoxPrefix{1}, 10, 12, "> "},
                Token{PlainText{"line"}, 12, 16, "line"},
            });
        });
        
        tester.add("Parser::tokenizePage ListPrefix", [](){
            assertPageTokenize("* list", {
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 0, 2, "* "},
                Token{PlainText{"list"}, 2, 6, "list"}
            });
            assertPageTokenize("* list\n* bullets", {
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 0, 2, "* "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 7, 9, "* "},
                Token{PlainText{"bullets"}, 9, 16, "bullets"}
            });
            assertPageTokenize("* list\n * bullets\n  * yeah\n* l * s", {
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 0, 2, "* "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 2}, 7, 10, " * "},
                Token{PlainText{"bullets"}, 10, 17, "bullets"},
                Token{NewLine{}, 17, 18, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 3}, 18, 22, "  * "},
                Token{PlainText{"yeah"}, 22, 26, "yeah"},
                Token{NewLine{}, 26, 27, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 27, 29, "* "},
                Token{PlainText{"l * s"}, 29, 34, "l * s"}
            });
            
            assertPageTokenize("# list", {
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 0, 2, "# "},
                Token{PlainText{"list"}, 2, 6, "list"}
            });
            assertPageTokenize("# list\n* bullets", {
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 0, 2, "# "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 7, 9, "* "},
                Token{PlainText{"bullets"}, 9, 16, "bullets"}
            });
            assertPageTokenize("# list\n # bullets\n  # yeah\n# l # s", {
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 0, 2, "# "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 2}, 7, 10, " # "},
                Token{PlainText{"bullets"}, 10, 17, "bullets"},
                Token{NewLine{}, 17, 18, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 3}, 18, 22, "  # "},
                Token{PlainText{"yeah"}, 22, 26, "yeah"},
                Token{NewLine{}, 26, 27, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 27, 29, "# "},
                Token{PlainText{"l # s"}, 29, 34, "l # s"}
            });
            
            assertPageTokenize("* list\n # one\n # two\n* lists", {
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 0, 2, "* "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 2}, 7, 10, " # "},
                Token{PlainText{"one"}, 10, 13, "one"},
                Token{NewLine{}, 13, 14, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 2}, 14, 17, " # "},
                Token{PlainText{"two"}, 17, 20, "two"},
                Token{NewLine{}, 20, 21, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 21, 23, "* "},
                Token{PlainText{"lists"}, 23, 28, "lists"}
            });
            assertPageTokenize("# list\n * one\n * two\n# lists", {
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 0, 2, "# "},
                Token{PlainText{"list"}, 2, 6, "list"},
                Token{NewLine{}, 6, 7, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 2}, 7, 10, " * "},
                Token{PlainText{"one"}, 10, 13, "one"},
                Token{NewLine{}, 13, 14, "\n"},
                Token{ListPrefix{ListPrefix::Type::Bullet, 2}, 14, 17, " * "},
                Token{PlainText{"two"}, 17, 20, "two"},
                Token{NewLine{}, 20, 21, "\n"},
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 21, 23, "# "},
                Token{PlainText{"lists"}, 23, 28, "lists"}
            });
            
            assertPageTokenize("> * list", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{ListPrefix{ListPrefix::Type::Bullet, 1}, 2, 4, "* "},
                Token{PlainText{"list"}, 4, 8, "list"}
            });
            assertPageTokenize(">>  * list", {
                Token{QuoteBoxPrefix{2}, 0, 3, ">> "},
                Token{ListPrefix{ListPrefix::Type::Bullet, 2}, 3, 6, " * "},
                Token{PlainText{"list"}, 6, 10, "list"}
            });
            
            assertPageTokenize("> # list", {
                Token{QuoteBoxPrefix{1}, 0, 2, "> "},
                Token{ListPrefix{ListPrefix::Type::Number, 1}, 2, 4, "# "},
                Token{PlainText{"list"}, 4, 8, "list"}
            });
            assertPageTokenize(">>  # list", {
                Token{QuoteBoxPrefix{2}, 0, 3, ">> "},
                Token{ListPrefix{ListPrefix::Type::Number, 2}, 3, 6, " # "},
                Token{PlainText{"list"}, 6, 10, "list"}
            });
        });
        
        tester.add("Parser::tokenizePage Divider", [](){
			assertPageTokenize("----\nyup", {
                Token{Divider{Divider::Type::Line}, 0, 4, "----"},
                Token{NewLine{}, 4, 5, "\n"},
                Token{PlainText{"yup"}, 5, 8, "yup"}
            });
            assertPageTokenize("-------\nyup", {
                Token{Divider{Divider::Type::Line}, 0, 7, "-------"},
                Token{NewLine{}, 7, 8, "\n"},
                Token{PlainText{"yup"}, 8, 11, "yup"}
            });
            assertPageTokenize("huh?\n------\nyup", {
				Token{PlainText{"huh?"}, 0, 4, "huh?"},
				Token{NewLine{}, 4, 5, "\n"},
                Token{Divider{Divider::Type::Line}, 5, 11, "------"},
                Token{NewLine{}, 11, 12, "\n"},
                Token{PlainText{"yup"}, 12, 15, "yup"}
            });
            
            assertPageTokenize("~~~~\nyup", {
                Token{Divider{Divider::Type::Clear}, 0, 4, "~~~~"},
                Token{NewLine{}, 4, 5, "\n"},
                Token{PlainText{"yup"}, 5, 8, "yup"}
            });
            assertPageTokenize("~~~~~~~\nyup", {
                Token{Divider{Divider::Type::Clear}, 0, 7, "~~~~~~~"},
                Token{NewLine{}, 7, 8, "\n"},
                Token{PlainText{"yup"}, 8, 11, "yup"}
            });
            assertPageTokenize("huh?\n~~~~~~\nyup", {
				Token{PlainText{"huh?"}, 0, 4, "huh?"},
				Token{NewLine{}, 4, 5, "\n"},
                Token{Divider{Divider::Type::Clear}, 5, 11, "~~~~~~"},
                Token{NewLine{}, 11, 12, "\n"},
                Token{PlainText{"yup"}, 12, 15, "yup"}
            });
		});
        
        tester.add("Parser::tokenizePage InlineFormat::Strike", [](){
            assertPageTokenize("A --B-- C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Strike, true, false}, 2, 4, "--"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Strike, false, true}, 5, 7, "--"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A-- B -- C --D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Strike, false, true}, 1, 3, "--"},
                Token{PlainText{" B — C "}, 3, 11, " B -- C "},
                Token{InlineFormat{InlineFormat::Type::Strike, true, false}, 11, 13, "--"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Italics", [](){
            assertPageTokenize("A //B// C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Italics, true, false}, 2, 4, "//"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Italics, false, true}, 5, 7, "//"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A// B // C //D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Italics, false, true}, 1, 3, "//"},
                Token{PlainText{" B // C "}, 3, 11, " B // C "},
                Token{InlineFormat{InlineFormat::Type::Italics, true, false}, 11, 13, "//"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Bold", [](){
            assertPageTokenize("A **B** C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Bold, true, false}, 2, 4, "**"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Bold, false, true}, 5, 7, "**"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A** B ** C **D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Bold, false, true}, 1, 3, "**"},
                Token{PlainText{" B ** C "}, 3, 11, " B ** C "},
                Token{InlineFormat{InlineFormat::Type::Bold, true, false}, 11, 13, "**"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Underline", [](){
            assertPageTokenize("A __B__ C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Underline, true, false}, 2, 4, "__"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Underline, false, true}, 5, 7, "__"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A__ B __ C __D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Underline, false, true}, 1, 3, "__"},
                Token{PlainText{" B __ C "}, 3, 11, " B __ C "},
                Token{InlineFormat{InlineFormat::Type::Underline, true, false}, 11, 13, "__"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Super", [](){
            assertPageTokenize("A ^^B^^ C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Super, true, false}, 2, 4, "^^"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Super, false, true}, 5, 7, "^^"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A^^ B ^^ C ^^D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Super, false, true}, 1, 3, "^^"},
                Token{PlainText{" B ^^ C "}, 3, 11, " B ^^ C "},
                Token{InlineFormat{InlineFormat::Type::Super, true, false}, 11, 13, "^^"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Sub", [](){
            assertPageTokenize("A ,,B,, C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Sub, true, false}, 2, 4, ",,"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Sub, false, true}, 5, 7, ",,"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
            
            assertPageTokenize("A,, B ,, C ,,D", {
                Token{PlainText{"A"}, 0, 1, "A"},
                Token{InlineFormat{InlineFormat::Type::Sub, false, true}, 1, 3, ",,"},
                Token{PlainText{" B ,, C "}, 3, 11, " B ,, C "},
                Token{InlineFormat{InlineFormat::Type::Sub, true, false}, 11, 13, ",,"},
                Token{PlainText{"D"}, 13, 14, "D"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Monospace", [](){
            assertPageTokenize("A {{B}} C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Monospace, true, false}, 2, 4, "{{"},
                Token{PlainText{"B"}, 4, 5, "B"},
                Token{InlineFormat{InlineFormat::Type::Monospace, false, true}, 5, 7, "}}"},
                Token{PlainText{" C"}, 7, 9, " C"}
            });
        });
        
        tester.add("Parser::tokenizePage InlineFormat::Color", [](){
            assertPageTokenize("A ##blue|B## C", {
                Token{PlainText{"A "}, 0, 2, "A "},
                Token{InlineFormat{InlineFormat::Type::Color, true, false, "blue"}, 2, 9, "##blue|"},
                Token{PlainText{"B"}, 9, 10, "B"},
                Token{InlineFormat{InlineFormat::Type::Color, false, true}, 10, 12, "##"},
                Token{PlainText{" C"}, 12, 14, " C"}
            });
        });
	}
}
