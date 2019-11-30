#include "Treer.hpp"

#include <sstream>

#include "TreerRules.hpp"

namespace Parser{

    bool Size::operator==(const Size& nod)const{
        return size == nod.size;
    }
    
    bool Span::operator==(const Span& nod)const{
        return parameters == nod.parameters;
    }
	
	bool StyleFormat::operator==(const StyleFormat& nod)const{
		return nod.type == type && nod.color == color;
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
						default:
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
						case StyleFormat::Type::Color:
							ss << "Color," << format.color;
							break;
					}
					ss << "\"";
				}
				break;
            case Node::Type::Size:
				{
					const Size& size = std::get<Size>(nod.node);
					ss << "Size:\"" << size.size << "\"";
				}
				break;
            case Node::Type::Span:
				{
					const Span& span = std::get<Span>(nod.node);
                    ss << "Span:{";
                    for(auto i = span.parameters.begin(); i != span.parameters.end(); i++){
                        ss << i->first << ": " << i->second << ", ";
                    }
                    ss << "}";
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
	
    bool isTextType(SectionType type){
        switch(type){
            default:
                return false;
            case SectionType::Size:
            case SectionType::Span:
                return true;
        }
    }
    
    bool isTextType(const Token& tok){
        switch(tok.getType()){
            default:
                return false;
            case Token::Type::PlainText:
            case Token::Type::LiteralText:
            case Token::Type::InlineFormat:
            case Token::Type::HyperLink:
                return true;
            case Token::Type::Section:
                return isTextType(std::get<Section>(tok.token).type);
            case Token::Type::SectionStart:
                return isTextType(std::get<SectionStart>(tok.token).type);
            case Token::Type::SectionEnd:
                return isTextType(std::get<SectionEnd>(tok.token).type);
            case Token::Type::SectionComplete:
                return isTextType(std::get<SectionComplete>(tok.token).type);
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
            case Node::Type::Size:
            case Node::Type::Span:
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
            case Node::Type::Size:
            case Node::Type::Span:
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
        while(!isDivInsertable(context.stack.back().getType())){
            popStackWithCarry(context);
        }
    }
    
    void makeTextAddable(TreeContext& context){
        if(!isTextInsertable(context.stack.back().getType())){
            makeDivPushable(context);
            pushStack(context, Node{Paragraph{}});
        }
        while(context.styleCarry.size() > 0){
            pushStack(context, context.styleCarry.back());
            context.styleCarry.pop_back();
        }
    }
    
    void addAsDiv(TreeContext& context, Node newNode){
        makeDivPushable(context);
        context.stack.back().branches.push_back(newNode);
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
	
	namespace{
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
	}
	
	namespace{
        const std::vector<TreeRule> treeRules = {
            TreeRule{{Token::Type::InlineFormat}, handleInlineFormat},
            TreeRule{{Token::Type::PlainText}, handlePlainText},
            TreeRule{{Token::Type::LiteralText}, handleLiteralText},
            TreeRule{{Token::Type::LineBreak}, handleLineBreak},
            TreeRule{{Token::Type::HyperLink}, handleHyperLink},
            TreeRule{{Token::Type::InlineFormat}, handleInlineFormat},
            TreeRule{{Token::Type::Heading}, handleHeading},
            TreeRule{{Token::Type::Divider}, handleDivider},
            
            TreeRule{{Token::Type::Section, SectionType::Size}, handleSize},
            TreeRule{{Token::Type::Section, SectionType::Span}, handleSpan}
        };
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
				if(context.newlines == 1 && isTextType(token)){
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
			
			bool found = false;
			for(const TreeRule& rule : treeRules){
                constexpr auto getSectionTypes = [](const Token& token, SectionType& type, ModuleType& moduleType){
                    switch(token.getType()){
                        case Token::Type::Section:
                            {
                                const Section& section = std::get<Section>(token.token);
                                type = section.type;
                                moduleType = section.moduleType;
                            }
                            break;
                        case Token::Type::SectionStart:
                            {
                                const SectionStart& section = std::get<SectionStart>(token.token);
                                type = section.type;
                                moduleType = section.moduleType;
                            }
                            break;
                        case Token::Type::SectionComplete:
                            {
                                const SectionComplete& section = std::get<SectionComplete>(token.token);
                                type = section.type;
                                moduleType = section.moduleType;
                            }
                            break;
                        default:
                            throw new std::runtime_error("Entered impossible situation when getting section types");
                    }
                };
                
                Token::Type tokenType = token.getType();
                
                switch(tokenType){
                    default:
                        if(tokenType == rule.type.mainType){
                            rule.handleRule(context, token);
                            found = true;
                        }
                        break;
                    case Token::Type::Section:
                    case Token::Type::SectionStart:
                    case Token::Type::SectionComplete:
                        if(rule.type.mainType == Token::Type::Section){
                            SectionType sectionType;
                            ModuleType moduleType;
                            getSectionTypes(token, sectionType, moduleType);
                            if(sectionType == rule.type.sectionType){
                                if(sectionType == SectionType::Module){
                                    if(rule.type.moduleType == moduleType){
                                        rule.handleRule(context, token);
                                        found = true;
                                    }
                                }
                                else{
                                    rule.handleRule(context, token);
                                    found = true;
                                }
                            }
                        }
                        break;
                    case Token::Type::SectionEnd:
                        if(rule.type.mainType == Token::Type::Section && std::get<SectionEnd>(token.token).type == rule.type.sectionType){
                            rule.handleRule(context, token);
                            found = true;
                        }
                        break;
                }
                if(found){
                    break;
                }
			}
			if(!found){
                //throw std::runtime_error("Could not find handling rule for Token");
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





















