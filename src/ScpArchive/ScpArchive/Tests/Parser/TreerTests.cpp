#include "TreerTests.hpp"

#include "../../Parser/Treer.hpp"

namespace Tests{
	using namespace Parser;
	
	namespace{
		void assertPageTree(std::string source, Node node){
			PageTree page = makeTreeFromPage(source);
			assertEquals(node, page.pageRoot);
		}
	}
	
	void addTreerTests(Tester& tester){
		tester.add("Parser::makeTreeFromPage basicParagraphs", [](){
			assertPageTree(
			"Some text here\n"
			"\n"
			"more text\n",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"Some text here"}}
						}
					},
					Node{Paragraph{},
						{
							Node{PlainText{"more text"}}
						}
					}
				}
			});
			assertPageTree(
			"Some text here\n"
			"more text",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"Some text here"}},
							Node{LineBreak{}},
							Node{PlainText{"more text"}}
						}
					}
				}
			});
			assertPageTree(
			"Some text here _\n"
			" _\n"
			" _\n"
			"more text",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"Some text here"}},
							Node{LineBreak{}},
							Node{LineBreak{}},
							Node{LineBreak{}},
							Node{PlainText{"more text"}}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage HyperLink", [](){
			assertPageTree(
			"[[[Page Link]]]",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{HyperLink{"Page Link", "page-link", false}}
						}
					}
				}
			});
			assertPageTree(
			"here is the https://google.com/ link",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"here is the "}},
							Node{HyperLink{"https://google.com/", "https://google.com/", false}},
							Node{PlainText{" link"}}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage StyleFormat", [](){
			assertPageTree(
			"**bold text**",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{
								StyleFormat{StyleFormat::Type::Bold},
								{
									Node{PlainText{"bold text"}}
								}
							}
						}
					}
				}
			});
			
			assertPageTree(
			"**bold**text**",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{StyleFormat{StyleFormat::Type::Bold},
								{
									Node{PlainText{"bold"}}
								}
							},
							Node{PlainText{"text**"}}
						}
					}
				}
			});
			
			assertPageTree(
			"** bold**text**",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"** bold"}},
							Node{StyleFormat{StyleFormat::Type::Bold},
								{
									Node{PlainText{"text"}}
								}
							}
						}
					}
				}
			});
			
			assertPageTree(
			"A --B --C D-- E-- F",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"A "}},
							Node{StyleFormat{StyleFormat::Type::Strike},
								{
									Node{PlainText{"B —C D"}}
								}
							},
							Node{PlainText{" E— F"}}
						}
					}
				}
			});
			
			assertPageTree(
			"A ,,B ,,C D,, E,, F",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"A "}},
							Node{StyleFormat{StyleFormat::Type::Sub},
								{
									Node{PlainText{"B ,,C D"}}
								}
							},
							Node{PlainText{" E,, F"}}
						}
					}
				}
			});
			
			assertPageTree(
			"**bold\ntext**",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{StyleFormat{StyleFormat::Type::Bold},
								{
									Node{PlainText{"bold"}},
									Node{LineBreak{}},
									Node{PlainText{"text"}}
								}
							}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage Heading",[](){
			assertPageTree(
			"+++ heading",
			Node{
				RootPage{},
				{
					Node{Heading{3, false},
						{
							Node{PlainText{"heading"}}
						}
					}
				}
			});
			
			assertPageTree(
			"+* hidden heading",
			Node{
				RootPage{},
				{
					Node{Heading{1, true},
						{
							Node{PlainText{"hidden heading"}}
						}
					}
				}
			});
		});
	}
}
