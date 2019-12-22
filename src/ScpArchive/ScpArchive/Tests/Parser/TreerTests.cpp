#include "TreerTests.hpp"

#include "../../Parser/Treer.hpp"

namespace Tests{
	using namespace Parser;
	
	namespace{
		void assertPageTree(std::string source, Node node, std::vector<CSS> cssData = {}, std::vector<Code> codeData = {}){
			PageTree page = makeTreeFromPage(source);
			PageTree compare;
			compare.pageRoot = std::move(node);
			compare.cssData = std::move(cssData);
			compare.codeData = std::move(codeData);
			assertEquals(compare, page);
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
			assertPageTree(
			"Some text here\n"
			" \n"
			"  \n"
			" \n"
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
			"**double **begin",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"**double **begin"}}
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
			
			assertPageTree(
			"##blue|text##",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{StyleFormat{StyleFormat::Type::Color, "blue"},
								{
									Node{PlainText{"text"}}
								}
							}
						}
					}
				}
			});
			
			assertPageTree(
			"##blue|text##posttext##",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{StyleFormat{StyleFormat::Type::Color, "blue"},
								{
									Node{PlainText{"text"}}
								}
							},
							Node{PlainText{"posttext##"}}
						}
					}
				}
			});
			
			assertPageTree(
			"##blue|text## and then ##red|text##",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{StyleFormat{StyleFormat::Type::Color, "blue"},
								{
									Node{PlainText{"text"}}
								}
							},
							Node{PlainText{" and then "}},
							Node{StyleFormat{StyleFormat::Type::Color, "red"},
								{
									Node{PlainText{"text"}}
								}
							}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage StyleFormat Advanced", [](){
			assertPageTree(
			"A **B --C** D-- E",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"A "}},
							Node{StyleFormat{StyleFormat::Type::Bold},
								{
									Node{PlainText{"B "}},
									Node{StyleFormat{StyleFormat::Type::Strike},
										{
											Node{PlainText{"C"}}
										}
									}
								}
							},
							Node{StyleFormat{StyleFormat::Type::Strike},
								{
									Node{PlainText{" D"}}
								}
							},
							Node{PlainText{" E"}}
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
			"+++ heading\nnormal text",
			Node{
				RootPage{},
				{
					Node{Heading{3, false},
						{
							Node{PlainText{"heading"}}
						}
					},
					Node{Paragraph{},
						{
							Node{PlainText{"normal text"}}
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
			
			assertPageTree(
			"+ s--trik--e",
			Node{
				RootPage{},
				{
					Node{Heading{1, false},
						{
							Node{PlainText{"s"}},
							Node{StyleFormat{StyleFormat::Type::Strike},
								{
									Node{PlainText{"trik"}}
								}
							},
							Node{PlainText{"e"}}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage Divider",[](){
			assertPageTree(
			"before\n----",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::Line}}
				}
			});
			assertPageTree(
			"----\nafter",
			Node{
				RootPage{},
				{
					Node{Divider{Divider::Type::Line}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"before\n-----\nafter",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::Line}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"before\n------",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::Line}}
				}
			});
			assertPageTree(
			"----\nafter",
			Node{
				RootPage{},
				{
					Node{Divider{Divider::Type::Line}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"> before\n> ------\n> after",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"before"}}
								}
							},
							Node{Divider{Divider::Type::Line}},
							Node{Paragraph{},
								{
									Node{PlainText{"after"}}
								}
							}
						}
					}
				}
			});
			
			assertPageTree(
			"before\n~~~~",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::ClearBoth}}
				}
			});
			assertPageTree(
			"before\n~~~~<",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::ClearLeft}}
				}
			});
			assertPageTree(
			"before\n~~~~>",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::ClearRight}}
				}
			});
			assertPageTree(
			"~~~~\nafter",
			Node{
				RootPage{},
				{
					Node{Divider{Divider::Type::ClearBoth}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"before\n~~~~~~\nafter",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::ClearBoth}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"before\n~~~~",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"before"}}
						}
					},
					Node{Divider{Divider::Type::ClearBoth}}
				}
			});
			assertPageTree(
			"~~~~~\nafter",
			Node{
				RootPage{},
				{
					Node{Divider{Divider::Type::ClearBoth}},
					Node{Paragraph{},
						{
							Node{PlainText{"after"}}
						}
					}
				}
			});
			assertPageTree(
			"> before\n> ~~~~~~~~\n> after",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"before"}}
								}
							},
							Node{Divider{Divider::Type::ClearBoth}},
							Node{Paragraph{},
								{
									Node{PlainText{"after"}}
								}
							}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage LiteralText",[](){
			assertPageTree(
			"this @@is@@ some @<te&gt;&lt;t>@",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"this "}},
							Node{LiteralText{"is"}},
							Node{PlainText{" some "}},
							Node{LiteralText{"te><t"}}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage QuoteBox",[](){
			assertPageTree(
			"> simple quote",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"simple quote"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"good\n> quote",
			Node{
				RootPage{},
				{
                    Node{Paragraph{},
                        {
                            Node{PlainText{"good"}}
                        }
                    },
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"quote"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"> quote\n> part 2",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"quote"}},
									Node{LineBreak{}},
									Node{PlainText{"part 2"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			">> double quote",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{QuoteBox{},
								{
									Node{Paragraph{},
										{
											Node{PlainText{"double quote"}}
										}
									}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			">> double quote\n> drop to single\n> \n> new paragraph",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{QuoteBox{},
								{
									Node{Paragraph{},
										{
											Node{PlainText{"double quote"}}
										}
									}
								}
							},
							Node{Paragraph{},
								{
									Node{PlainText{"drop to single"}}
								}
							},
							Node{Paragraph{},
								{
									Node{PlainText{"new paragraph"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"none\n> single\n>> double\n> single again\nnone again\n",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"none"}}
						}
					},
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"single"}}
								}
							},
							Node{QuoteBox{},
								{
									Node{Paragraph{},
										{
											Node{PlainText{"double"}}
										}
									}
								}
							},
							Node{Paragraph{},
								{
									Node{PlainText{"single again"}}
								}
							}
						}
					},
					Node{Paragraph{},
						{
							Node{PlainText{"none again"}}
						}
					}
				}
			});
			assertPageTree(
			"> quote\n>\n> box",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"quote"}}
								}
							},
							Node{Paragraph{},
								{
									Node{PlainText{"box"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"> quote\n>  **too** many spaces\n> box",
			Node{
				RootPage{},
				{
					Node{QuoteBox{},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"quote"}},
									Node{LineBreak{}},
									Node{StyleFormat{StyleFormat::Type::Bold},
                                        {
                                            Node{PlainText{"too"}}
                                        }
									},
									Node{PlainText{" many spaces"}},
									Node{LineBreak{}},
									Node{PlainText{"box"}}
								}
							}
						}
					}
				}
			});
		});
		
		tester.add("Parser::makeTreeFromPage List",[](){
			assertPageTree(
			"* simple list\n* simple list #2",
			Node{
				RootPage{},
				{
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}}
								}
							},
							Node{ListElement{},
								{
									Node{PlainText{"simple list #2"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"paragraph #1\n* simple list\n* simple list #2\nparagraph #2",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"paragraph #1"}}
						}
					},
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}}
								}
							},
							Node{ListElement{},
								{
									Node{PlainText{"simple list #2"}}
								}
							}
						}
					},
					Node{Paragraph{},
						{
							Node{PlainText{"paragraph #2"}}
						}
					}
				}
			});
			assertPageTree(
			"paragraph #1\n\n* simple list\n\n* simple list #2\n\nparagraph #2",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"paragraph #1"}}
						}
					},
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}}
								}
							}
						}
					},
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list #2"}}
								}
							}
						}
					},
					Node{Paragraph{},
						{
							Node{PlainText{"paragraph #2"}}
						}
					}
				}
			});
			assertPageTree(
			"# simple list\n# simple list #2",
			Node{
				RootPage{},
				{
					Node{List{List::Type::Number},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}}
								}
							},
							Node{ListElement{},
								{
									Node{PlainText{"simple list #2"}}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"* simple list\n * simple list #2\n* simple list: the prequel",
			Node{
				RootPage{},
				{
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}},
									Node{List{List::Type::Bullet},
                                        {
                                            Node{ListElement{},
                                                {
                                                    Node{PlainText{"simple list #2"}}
                                                }
                                            }
                                        }
									}
								}
							},
                            Node{ListElement{},
                                {
                                    Node{PlainText{"simple list: the prequel"}}
                                }
                            }
						}
					}
				}
			});
			assertPageTree(
			"* simple list\n  * skip",
			Node{
				RootPage{},
				{
					Node{List{List::Type::Bullet},
						{
							Node{ListElement{},
								{
									Node{PlainText{"simple list"}},
									Node{List{List::Type::Bullet},
                                        {
                                            Node{ListElement{},
                                                {
                                                    Node{List{List::Type::Bullet},
                                                        {
                                                            Node{ListElement{},
                                                                {
                                                                    Node{PlainText{"skip"}}
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
									}
								}
							}
						}
					}
				}
			});
			assertPageTree(
			"> * simple list\n>  * simple list #2\n> * simple list: the prequel\n> yeah\nyeah #2",
			Node{
				RootPage{},
				{
                    Node{QuoteBox{},
                        {
                            Node{List{List::Type::Bullet},
                                {
                                    Node{ListElement{},
                                        {
                                            Node{PlainText{"simple list"}},
                                            Node{List{List::Type::Bullet},
                                                {
                                                    Node{ListElement{},
                                                        {
                                                            Node{PlainText{"simple list #2"}}
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    },
                                    Node{ListElement{},
                                        {
                                            Node{PlainText{"simple list: the prequel"}}
                                        }
                                    }
                                }
                            },
                            Node{Paragraph{},
                                {
                                    Node{PlainText{"yeah"}}
                                }
                            }
                        }
                    },
                    Node{Paragraph{},
                        {
                            Node{PlainText{"yeah #2"}}
                        }
                    }
				}
			});
        });
        
        tester.add("Parser::makeTreeFromPage Section",[](){
			assertPageTree(
			"[[size 300%]]big text[[/size]]",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{Size{"300%"},
								{
									Node{PlainText{"big text"}}
								}
							}
						}
					}
				}
			});
        });
        
        tester.add("Parser::makeTreeFromPage Div",[](){
			assertPageTree(
			"[[diV style=\"color:blue;\"]]\nhello\n[[/Div]]",
			Node{
				RootPage{},
				{
					Node{Div{{{"style", "color:blue;"}}},
						{
							Node{Paragraph{},
								{
									Node{PlainText{"hello"}}
								}
							}
						}
					}
				}
			});
        });
        
        tester.add("Parser::makeTreeFromPage CSS",[](){
			assertPageTree(
			"[[module Css]]\n.test{color:black;}\n[[/module]]\nhello",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{PlainText{"hello"}}
						}
					}
				}
			},
			{
				CSS{"\n.test{color:black;}\n"}
			});
        });
        
        tester.add("Parser::makeTreeFromPage Anchor",[](){
			assertPageTree(
			"[[# yeah]]you know?",
			Node{
				RootPage{},
				{
					Node{Paragraph{},
						{
							Node{Anchor{"yeah"}},
							Node{PlainText{"you know?"}}
						}
					}
				}
			});
        });
        
        tester.add("Parser::makeTreeFromPage Collapsible",[](){
			assertPageTree(
			"[[collapsible]]\nyes\n[[/collapsible]]",
			Node{
				RootPage{},
				{
				    Node{Collapsible{"+ show block", "- hide block", false},
                        {
                            Node{Paragraph{},
                                {
                                    Node{PlainText{"yes"}}
                                }
                            }
                        }
					}
				}
			});
			
			assertPageTree(
			"[[collapsible show=\"A\" hide=\"B\" folded=\"no\"]]\nyes\n[[/collapsible]]",
			Node{
				RootPage{},
				{
				    Node{Collapsible{"A", "B", true},
                        {
                            Node{Paragraph{},
                                {
                                    Node{PlainText{"yes"}}
                                }
                            }
                        }
					}
				}
			});
        });
        
        ///TODO: write test for include(going to be annoying) and images(not that bad but still annoying)
        
        tester.add("Parser::makeTreeFromPage Code",[](){
			assertPageTree(
			"yeah\n[[code type=\"CsS\"]]\nsome code...\n[[/code]]",
			Node{
				RootPage{},
				{
                    Node{Paragraph{},
                        {
                            Node{PlainText{"yeah"}}
                        }
                    },
                    Node{Code{"\nsome code...\n"}}
				}
			}, {}, {Code{"\nsome code...\n", "css"}});
        });
        
        tester.add("Parser::makeTreeFromPage CenterText",[](){
			assertPageTree(
			"yeah\n= this\nworks",
			Node{
				RootPage{},
				{
                    Node{Paragraph{},
                        {
                            Node{PlainText{"yeah"}}
                        }
                    },
                    Node{CenterText{},
                        {
                            Node{PlainText{"this"}}
                        }
                    },
                    Node{Paragraph{},
                        {
                            Node{PlainText{"works"}}
                        }
                    }
				}
			});
        });
	}
}





















