#include "Treer.hpp"

#include <sstream>

namespace Parser{
	
	bool StyleFormat::operator==(const StyleFormat& nod)const{
		return nod.type == type;
	}
	
	bool Paragraph::operator==(const Paragraph& nod)const{
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
			case Node::Type::Paragraph:
				ss << "Paragraph";
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
                case Node::Type::StyleFormat:
                    return true;
            }
		}
		
		bool isDivInsertable(Node::Type type){
            switch(type){
                default:
                    return false;
                case Node::Type::RootPage:
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
		
		void popSingle(TreeContext& context, std::function<bool(const Node&)> check){
			while(true){
                const auto& back = context.stack.back();
                if(check(back)){
                    popStack(context);
                    break;
                }
                else if(isStyleCarryable(back.getType())){
                    Node style = back;
                    style.branches.clear();
                    context.styleCarry.push_back(style);//don't copy over branches, just copy
                    popStack(context);
                }
                else{
                    popStack(context);
                }
			}
		}
		
		void popSingle(TreeContext& context, Node::Type type){
            popSingle(context, [type](const Node& nod){return type == nod.getType();});
		}
		
		void makeDivPushable(TreeContext& context){
			while(!isInsideDivInsertable(context)){
				popStack(context);
			}
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
			}
			else if(!alreadyInStyle && tokenFormat.begin){
				makeTextAddable(context);
				pushStack(context, Node{StyleFormat{tokenFormat.type}});
			}
			else{
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
	}
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage){
		PageTree page;
		
		TreeContext context;
		context.stack.push_back(Node{RootPage{}});//start out with a page root
		context.newlines = 0;
		
		for(const Token& token : tokenPage.tokens){
			if(token.getType() == Token::Type::NewLine){
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
			if(isInside(context, Node::Type::Heading)){
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





















