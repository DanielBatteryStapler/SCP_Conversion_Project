#include "Treer.hpp"

#include "Rules/RuleSet.hpp"

#include <sstream>

namespace Parser{
	nlohmann::json printShownAuthor(const ShownAuthor& author){
		nlohmann::json out;
		switch(author.type){
			default:
				throw std::runtime_error("Attempted to print ShownAuthor with invalid Type");
				break;
			case ShownAuthor::Type::System:
				out["type"] = "System";
				break;
			case ShownAuthor::Type::User:
				out["type"] = "User";
				break;
			case ShownAuthor::Type::Deleted:
				out["type"] = "Deleted";
				break;
		}
		out["shownName"] = author.shownName;
		out["linkName"] = author.linkName;
		return out;
	}
	
	bool Code::operator==(const Code& code)const{
		return contents == code.contents && type == code.type;
	}
	
	Node::Type Node::getType()const{
		return static_cast<Type>(node.index());
	}
	
	bool Node::operator==(const Node& nod)const{
		return printNode(*this) == printNode(nod);
	}
	
	std::string getNodeTypeName(Node::Type type){
		return NodeTypeNames.at(static_cast<std::size_t>(type));
	}
	
	nlohmann::json printNodeVariant(const Node& nod){
		const std::vector<NodePrintRule> nodePrintRules = getNodePrintRules();
		Node::Type nodType = nod.getType();
	
		for(const NodePrintRule& printRule : nodePrintRules){
			if(printRule.type == nodType){
				return printRule.print(nod.node);
			}
		}
		throw std::runtime_error("Attempted to print a Node with no valid NodePrintRule");
	}
	
	nlohmann::json printNode(const Node& nod){
		nlohmann::json out;
		out["type"] = getNodeTypeName(nod.getType());
		out["data"] = printNodeVariant(nod);
		out["branches"] = nlohmann::json::array();
		for(const Node& i : nod.branches){
			out["branches"].push_back(printNode(i));
		}
		return out;
	}
	
	std::ostream& operator<<(std::ostream& out, const Node& nod){
		return out << printNode(nod).dump(4);
	}
	
	bool CSS::operator==(const CSS& css)const{
		return data == css.data;
	}
	
	bool PageTree::operator==(const PageTree& page)const{
		return page.pageRoot == pageRoot && page.cssData == cssData && page.codeData == codeData;
	}
	
	std::ostream& operator<<(std::ostream& out, const PageTree& page){
		out << page.pageRoot;
		out << "CSS{\n";
		for(const CSS& css : page.cssData){
			out << "    " << css.data << ",\n";
		}
		out << "}\n";
		out << "Code{\n";
		for(const Code& code : page.codeData){
			out << "    " << code.type << " -> " << code.contents << ",\n";
		}
		out << "}\n";
		return out;
	}
	
    bool isTextType(SectionType type){
        switch(type){
            default:
                return false;
            case SectionType::Size:
            case SectionType::Span:
			case SectionType::Anchor:
            case SectionType::FootNote:
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
            case Token::Type::LineBreak:
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
            case Node::Type::CenterText:
            case Node::Type::Heading:
            case Node::Type::ListElement:
            case Node::Type::StyleFormat:
            case Node::Type::Size:
            case Node::Type::Span:
            case Node::Type::TableElement:
			case Node::Type::A:
                return true;
        }
    }
    
    bool isDivInsertable(Node::Type type){
        switch(type){
            default:
                return false;
            case Node::Type::Align:
            case Node::Type::RootPage:
            case Node::Type::QuoteBox:
			case Node::Type::Div:
            case Node::Type::Collapsible:
            case Node::Type::Tab:
            case Node::Type::FootNote:
			case Node::Type::AdvTableElement:
			case Node::Type::AdvListElement:
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
        for(auto i = context.stack.rbegin(); i != context.stack.rend(); i++){
            if(i->getType() == type){
                return true;
            }
            if(i->getType() == Node::Type::Collapsible || i->getType() == Node::Type::FootNote){
                ///isinside's "vision" is blocked by some kinds of nodes
                ///e.g. it is impossible to end a [[=]] inside of a [[collapsible]]
                return false;
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
	
	void makeTop(TreeContext& context, Node::Type type){
		while(context.stack.back().getType() != type){
			popStackWithCarry(context);
		}
	}
	
	namespace{
        void handleQuoteBoxNesting(TreeContext& context, std::size_t pos){
            const auto countQuoteBoxes = [](TreeContext& context)->unsigned int{
                unsigned int count = 0;
                for(auto i = context.stack.rbegin(); i != context.stack.rend(); i++){
                    if(i->getType() == Node::Type::QuoteBox){
                        count++;
                    }
                    else if(i->getType() == Node::Type::FootNote){
                        break;//quotebox stuff can't penetrate inside of a footnote
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
                for(auto i = context.stack.rbegin(); i != context.stack.rend(); i++){
                    if(i->getType() == Node::Type::List){
                        count++;
                    }
                    else if(i->getType() == Node::Type::FootNote){
                        break;//list stuff can't penetrate inside of a footnote
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
        
        void handleTable(TreeContext& context, int newlines){
            if(isInside(context, Node::Type::Table)){
                if(newlines > 0){
                    //don't popStack(context) the TableElement off, literally delete the TableElement and all of its branches
                    //(I'm not joking, this is really what wikidot does)
                    if(isInside(context, Node::Type::TableElement)){
                        while(context.stack.back().getType() != Node::Type::TableElement){
                            popStack(context);
                        }
                        context.stack.pop_back();
                        popSingle(context,Node::Type::TableRow);//and then pop the row off
                    }
                }
                if(newlines > 1){
                    popSingle(context, Node::Type::Table);
                }
			}
        }
	}
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage, ParserParameters parameters){
		PageTree page;
		
		TreeContext context;
		context.parameters = parameters;
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
			
			if(token.getType() == Token::Type::LineBreak){
				if(context.tokenedPage.tokens.size() > context.tokenPos + 1){
					handleQuoteBoxNesting(context, context.tokenPos + 1);
					handleListNesting(context, context.tokenPos + 1);
				}
			}
			
			if(isInside(context, Node::Type::Paragraph)){
				if(context.newlines == 1 && isTextType(token)){
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
            if(isInside(context, Node::Type::CenterText)){
				if(context.newlines > 0){
					popSingle(context, Node::Type::CenterText);
				}
			}
			handleTable(context, context.newlines);
            
			
			const std::vector<TreeRule> treeRules = getTreeRules();
			
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
                throw std::runtime_error("Could not find handling rule for Token");
			}
		}
		
        handleTable(context, 1);//this is to handle a weird edge case where a table needs to ignore the very last element
		
		while(context.stack.size() > 1){
			popStack(context);
		}
		
		{
            std::vector<PostTreeRule> postTreeRules = getPostTreeRules();
            for(const PostTreeRule& rule : postTreeRules){
                rule.rule(context);
            }
		}
		
		page.pageRoot = std::move(context.stack[0]);
		page.cssData = std::move(context.cssData);
		page.codeData = std::move(context.codeData);
		
		return page;
	}
	
	PageTree makeTreeFromPage(std::string page, ParserParameters parameters){
		return makeTreeFromTokenedPage(tokenizePage(page, parameters), parameters);
	}
}





















