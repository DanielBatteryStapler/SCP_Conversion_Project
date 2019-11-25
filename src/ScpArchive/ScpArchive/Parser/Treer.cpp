#include "Treer.hpp"

#include <sstream>

namespace Parser{
	
	bool StyleFormat::operator==(const StyleFormat& nod)const{
		return nod.type == type;
	}
	
	bool Paragraph::operator==(const Paragraph& nod)const{
		return true;
	}
	
	
    bool List::operator==(const List& nod)const{
        return type == nod.type;
    }
		
    bool ListElement::operator==(const ListElement& nod)const{
        return true;
    }
	
	bool QuoteBox::operator==(const QuoteBox& nod)const{
		return true;
	}
	
	bool RootPage::operator==(const RootPage& nod)const{
		return true;
	}
	
	Node::Type Node::getType()const{
		return static_cast<Type>(node.index());
	}
	
	bool Node::operator==(const Node& nod)const{
		return node == nod.node && branches == nod.branches;
	}
	
	std::string toString(const Node& nod, int tab){
		std::stringstream ss;
		const auto printTabs = [&ss, &tab](){
			for(int i = 0; i < tab; i++){
				ss << '\t';
			}
		};
		
		printTabs();
		switch(nod.getType()){
			case Node::Type::Unknown:
				ss << "Unknown";
				break;
			case Node::Type::RootPage:
				ss << "RootPage";
				break;
			case Node::Type::QuoteBox:
				ss << "QuoteBox";
				break;
            case Node::Type::List:
				{
					const List& list = std::get<List>(nod.node);
					ss << "List:\"";
					switch(list.type){
						case List::Type::Unknown:
							ss << "Unknown";
							break;
						case List::Type::Bullet:
							ss << "Bullet";
							break;
						case List::Type::Number:
							ss << "Number";
							break;
                        }
					ss << "\"";
				}
				break;
            case Node::Type::ListElement:
				ss << "ListElement";
				break;
			case Node::Type::Paragraph:
				ss << "Paragraph";
				break;
			case Node::Type::Divider:
				{
					const Divider& divider = std::get<Divider>(nod.node);
                    ss << "Divider:";
                    switch(divider.type){
					default:
						ss << "Unknown";
						break;
					case Divider::Type::Line:
						ss << "Line";
						break;
					case Divider::Type::Clear:
						ss << "Clear";
						break;
                    }
				}
				break;
			case Node::Type::Heading:
                {
					const Heading& heading = std::get<Heading>(nod.node);
                    ss << "Heading:" << heading.degree << ", " << (heading.hidden?"true":"false");
				}
				break;
			case Node::Type::LineBreak:
				ss << "LineBreak";
				break;
			case Node::Type::PlainText:
				ss << "PlainText:\"" << std::get<PlainText>(nod.node).text << "\"";
				break;
			case Node::Type::LiteralText:
				ss << "LiteralText:\"" << std::get<LiteralText>(nod.node).text << "\"";
				break;
			case Node::Type::HyperLink:
				{
					const HyperLink& link = std::get<HyperLink>(nod.node);
					ss << "HyperLink:\"" << link.shownText << "\", \"" << link.url << "\", " << (link.newWindow?"true":"false");
				}
				break;
			case Node::Type::StyleFormat:
				{
					const StyleFormat& format = std::get<StyleFormat>(nod.node);
					ss << "StyleFormat:\"";
					switch(format.type){
						case StyleFormat::Type::Unknown:
							ss << "Unknown";
							break;
						case StyleFormat::Type::Bold:
							ss << "Bold";
							break;
						case StyleFormat::Type::Italics:
							ss << "Italics";
							break;
						case StyleFormat::Type::Strike:
							ss << "Strike";
							break;
						case StyleFormat::Type::Underline:
							ss << "Underline";
							break;
						case StyleFormat::Type::Super:
							ss << "Super";
							break;
						case StyleFormat::Type::Sub:
							ss << "Sub";
							break;
					}
					ss << "\"";
				}
				break;
		}
		ss << "\n";
		
		for(const Node& i : nod.branches){
			ss << toString(i, tab + 1);
		}
		
		return ss.str();
	}
	
	std::ostream& operator<<(std::ostream& out, const Node& nod){
		return out << toString(nod);
	}
	
	namespace{
		struct TreeContext{
			std::vector<Node> stack;
			std::vector<Node> styleCarry;
			int newlines;
			std::size_t tokenPos;
			TokenedPage tokenedPage;
		};
		
		bool isTextType(Token::Type type){
			switch(type){
				default:
					return false;
				case Token::Type::PlainText:
				case Token::Type::LiteralText:
				case Token::Type::InlineFormat:
				case Token::Type::HyperLink:
					return true;
			}
		}
		
		bool isTextInsertable(Node::Type type){
            switch(type){
                default:
                    return false;
                case Node::Type::Paragraph:
                case Node::Type::Heading:
                case Node::Type::ListElement:
                case Node::Type::StyleFormat:
                    return true;
            }
		}
		
		bool isDivInsertable(Node::Type type){
            switch(type){
                default:
                    return false;
                case Node::Type::RootPage:
				case Node::Type::QuoteBox:
                    return true;
            }
		}
        
        bool isStyleCarryable(Node::Type type){
            switch(type){
                default:
                    return false;
                case Node::Type::StyleFormat:
                    return true;
            }
        }
        
		bool isInside(TreeContext& context, Node::Type type){
			for(const Node& i : context.stack){
				if(i.getType() == type){
					return true;
				}
			}
			return false;
		}
		
		bool isInsideTextInsertable(TreeContext& context){
			return isTextInsertable(context.stack.back().getType());
		}
		
		bool isInsideDivInsertable(TreeContext& context){
            return isDivInsertable(context.stack.back().getType());
		}
		
		void pushStack(TreeContext& context, Node newNode){
			context.stack.push_back(newNode);
		}
		
		void popStack(TreeContext& context){
			Node node = context.stack.back();
			context.stack.pop_back();
			context.stack.back().branches.push_back(node);
		}
		
		void popStackWithCarry(TreeContext& context){
            const auto& back = context.stack.back();
            if(isStyleCarryable(back.getType())){
                Node style = back;
                style.branches.clear();
                context.styleCarry.push_back(style);//don't copy over branches, just copy
                popStack(context);
            }
            else{
                popStack(context);
            }
		}
		
		void popSingle(TreeContext& context, std::function<bool(const Node&)> check){
			while(true){
                const auto& back = context.stack.back();
                if(check(back)){
                    popStack(context);
                    break;
                }
                else{
                    popStackWithCarry(context);
                }
			}
		}
		
		void popSingle(TreeContext& context, Node::Type type){
            popSingle(context, [type](const Node& nod){return type == nod.getType();});
		}
		
		void makeDivPushable(TreeContext& context){
			while(!isInsideDivInsertable(context)){
				popStackWithCarry(context);
			}
		}
		
		void addAsDiv(TreeContext& context, Node newNode){
			makeDivPushable(context);
			context.stack.back().branches.push_back(newNode);
		}
		
		void makeTextAddable(TreeContext& context){
			if(!isInsideTextInsertable(context)){
				while(!isInsideDivInsertable(context)){
                    popStack(context);
                }
				pushStack(context, Node{Paragraph{}});
			}
			while(context.styleCarry.size() > 0){
                pushStack(context, context.styleCarry.back());
                context.styleCarry.pop_back();
            }
		}
		
		void addAsText(TreeContext& context, Node newNode){
			makeTextAddable(context);
			//if there is already a	PlainText Token in the branch, just merge with that one
			if(newNode.getType() == Node::Type::PlainText && context.stack.back().branches.size() > 0
					&& context.stack.back().branches.back().getType() == Node::Type::PlainText){
				std::get<PlainText>(context.stack.back().branches.back().node).text += std::get<PlainText>(newNode.node).text;
			}
			else{
				context.stack.back().branches.push_back(newNode);
			}
		}
		
		void handleInlineFormat(TreeContext& context, const Token& token){
			const InlineFormat& tokenFormat = std::get<InlineFormat>(token.token);
			
			bool alreadyInStyle = false;
			for(const Node& i : context.stack){
				if(i.getType() == Node::Type::StyleFormat){
					const StyleFormat& format = std::get<StyleFormat>(i.node);
					if(format.type == tokenFormat.type){
						alreadyInStyle = true;
						break;
					}
				}
			}
			if(alreadyInStyle && tokenFormat.end){
				popSingle(context, [tokenFormat](const Node& nod){
                    if(nod.getType() == Node::Type::StyleFormat){
                        const StyleFormat& format = std::get<StyleFormat>(nod.node);
                        if(tokenFormat.type == format.type){
                            return true;
                        }
                    }
                    return false;
                });
                return;
			}
			else if(!alreadyInStyle && tokenFormat.begin){
				//there needs to be a valid end token too, so let's check for that
				bool hasValidEnd = false;
				{
					std::size_t pos = context.tokenPos + 1;
					bool wasNewline = false;
					while(pos < context.tokenedPage.tokens.size()){
						const Token& checkToken = context.tokenedPage.tokens[pos];
						if(checkToken.getType() == Token::Type::InlineFormat){
							const InlineFormat& inlineFormat = std::get<InlineFormat>(checkToken.token);
							if(tokenFormat.type == inlineFormat.type && inlineFormat.end){
								hasValidEnd = true;
								break;
							}
						}
						else if(checkToken.getType() == Token::Type::NewLine){
							if(wasNewline){
								break;
							}
							else{
								wasNewline = true;
							}
						}
						else{
							wasNewline = false;
						}
						pos++;
					}
				}
				if(hasValidEnd){
					makeTextAddable(context);
					pushStack(context, Node{StyleFormat{tokenFormat.type}});
					return;
				}
			}
			//this format marker doesn't line up with starting or stopping anything, so that means it is
			//"degenerate"(in the mathematical sense) and should go back into plain text
			switch(tokenFormat.type){
				default:
					addAsText(context, Node{PlainText{token.source}});
					break;
				case InlineFormat::Type::Strike:
					addAsText(context, Node{PlainText{"—"}});
					break;
			}
		}
	}
	
	void handleQuoteBoxNesting(TreeContext& context, std::size_t pos){
		const auto countQuoteBoxes = [](TreeContext& context)->unsigned int{
			unsigned int count = 0;
			for(const Node& node : context.stack){
				if(node.getType() == Node::Type::QuoteBox){
					count++;
				}
			}
			return count;
		};
		
		unsigned int wantedCount = 0;
		if(context.tokenedPage.tokens[pos].getType() == Token::Type::QuoteBoxPrefix){
			context.tokenPos++;
			const QuoteBoxPrefix& quoteBoxPrefix = std::get<QuoteBoxPrefix>(context.tokenedPage.tokens[pos].token);
			wantedCount = quoteBoxPrefix.degree;
		}
		while(true){
			unsigned int count = countQuoteBoxes(context);
			if(wantedCount > count){
				makeDivPushable(context);
				pushStack(context, Node{QuoteBox{}});
			}
			else if(wantedCount < count){
				popSingle(context, Node::Type::QuoteBox);
			}
			else{
				break;
			}
		}
	}
	
	void handleListNesting(TreeContext& context, std::size_t pos){
        const auto countLists = [](TreeContext& context)->unsigned int{
            unsigned int count = 0;
            for(const Node& node : context.stack){
				if(node.getType() == Node::Type::List){
					count++;
				}
			}
			return count;
        };
        
        List::Type wantedType = List::Type::Unknown;
        unsigned int wantedCount = 0;
        if(context.tokenedPage.tokens[pos].getType() == Token::Type::ListPrefix){
			context.tokenPos++;
			const ListPrefix& listPrefix = std::get<ListPrefix>(context.tokenedPage.tokens[pos].token);
			wantedType = listPrefix.type;
			wantedCount = listPrefix.degree;
		}
		
		unsigned int count = countLists(context);
		if(wantedCount == count){
            if(count > 0){
                popSingle(context, Node::Type::ListElement);
                pushStack(context, Node{ListElement{}});
            }
		}
        else{
            if(count > 0){
                while(context.stack.back().getType() != Node::Type::ListElement){
                    popStackWithCarry(context);
                }
            }
            else{
                makeDivPushable(context);
            }
            
            if(wantedCount > count){
                while(true){
                    count = countLists(context);
                    if(wantedCount == count){
                        break;
                    }
                    pushStack(context, Node{List{wantedType}});
                    pushStack(context, Node{ListElement{}});
                }
            }
            else{
                while(true){
                    count = countLists(context);
                    if(wantedCount == count){
                        break;
                    }
                    popSingle(context, Node::Type::List);
                    if(count > 1){
                        popSingle(context, Node::Type::ListElement);
                    }
                }
                if(wantedCount > 0){
                    pushStack(context, Node{ListElement{}});
                }
            }
		}
	}
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage){
		PageTree page;
		
		TreeContext context;
		context.stack.push_back(Node{RootPage{}});//start out with a page root
		context.newlines = 99;//act like we're already in a bunch of newlines
		context.tokenedPage = std::move(tokenPage);
		context.tokenPos = 0;
		
		if(context.tokenedPage.tokens.size() > 0) {
			handleQuoteBoxNesting(context, context.tokenPos);//needs to run in case of quoteboxes immediately at start
            handleListNesting(context, context.tokenPos);
		}
		
		for(; context.tokenPos < context.tokenedPage.tokens.size(); context.tokenPos++){
			const Token& token = context.tokenedPage.tokens[context.tokenPos];
			
			if(token.getType() == Token::Type::NewLine){
				if(context.tokenedPage.tokens.size() > context.tokenPos + 1){
					handleQuoteBoxNesting(context, context.tokenPos + 1);
					handleListNesting(context, context.tokenPos + 1);
				}
				context.newlines++;
				continue;
			}
			
			if(isInside(context, Node::Type::Paragraph)){
				if(context.newlines == 1 && isTextType(token.getType())){
					addAsText(context, Node{LineBreak{}});
				}
				else if(context.newlines >= 2){
					popSingle(context, Node::Type::Paragraph);
				}
			}
			else if(isInside(context, Node::Type::Heading)){
				if(context.newlines > 0){
					popSingle(context, Node::Type::Heading);
				}
			}
			
			context.newlines = 0;
			
			switch(token.getType()){
				default:
					throw std::runtime_error("Encountered Unknown/Unimplemented when constructing tree from page");
				case Token::Type::PlainText:
					addAsText(context, Node{std::get<PlainText>(token.token)});
					break;
				case Token::Type::LiteralText:
					addAsText(context, Node{std::get<LiteralText>(token.token)});
					break;
				case Token::Type::LineBreak:
					addAsText(context, Node{std::get<LineBreak>(token.token)});
					break;
				case Token::Type::HyperLink:
					addAsText(context, Node{std::get<HyperLink>(token.token)});
					break;
				case Token::Type::InlineFormat:
					handleInlineFormat(context, token);
					break;
				case Token::Type::Heading:
					makeDivPushable(context);
					pushStack(context, Node{std::get<Heading>(token.token)});
					break;
				case Token::Type::Divider:
					addAsDiv(context, Node{std::get<Divider>(token.token)});
					break;
			}
			
		}
		
		while(context.stack.size() > 1){
			popStack(context);
		}
		
		page.pageRoot = context.stack[0];
		
		return page;
	}
	
	PageTree makeTreeFromPage(std::string page){
		return makeTreeFromTokenedPage(tokenizePage(page));
	}
}





















