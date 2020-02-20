#include "ListRuleSet.hpp"

#include <sstream>

namespace Parser{
    std::string toStringTokenListPrefix(const TokenVariant& tok){
        const ListPrefix& listPrefix = std::get<ListPrefix>(tok);
        std::stringstream ss;
        ss << "ListPrefix:";
        switch(listPrefix.type){
        default:
            ss << "Unknown";
            break;
        case ListPrefix::Type::Bullet:
            ss << "Bullet";
            break;
        case ListPrefix::Type::Number:
            ss << "Number";
            break;
        }
        ss << ", " << listPrefix.degree;
        return ss.str();
    }
    
    std::string toStringNodeList(const NodeVariant& nod){
        const List& list = std::get<List>(nod);
        std::stringstream ss;
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
        return ss.str();
    }
    
    std::string toStringNodeListElement(const NodeVariant& nod){
        return "ListElement";
    }
    
    std::string toStringNodeAdvList(const NodeVariant& nod){
		const AdvList& advList = std::get<AdvList>(nod);
        std::string output = "AdvList:{";
        for(auto i = advList.parameters.begin(); i != advList.parameters.end(); i++){
            output += i->first + ": " + i->second + ", ";
        }
        output += "}";
        return output;
    }
    
    std::string toStringNodeAdvListElement(const NodeVariant& nod){
        const AdvListElement& advListElement = std::get<AdvListElement>(nod);
        std::string output = "AdvListElement:{";
        for(auto i = advListElement.parameters.begin(); i != advListElement.parameters.end(); i++){
            output += i->first + ": " + i->second + ", ";
        }
        output += "}";
        return output;
    }
    
	bool tryListPrefixRule(const TokenRuleContext& context){
        if(context.wasNewLine){
            std::size_t pos = context.pagePos;
			while(pos < context.page.size()){
				if(context.page[pos] == ' '){
					//keep going
				}
				else if(check(context.page, pos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
                    pos++;//the unicode space is 2 bytes long so we need to add in an extra increment
                    //keep going
				}
				else if(check(context.page, pos, "* ")){
                    return true;
				}
				else if(check(context.page, pos, "# ")){
                    return true;
				}
				else{
					return false;//fail if there is an unexpected character
				}
				pos++;
			}
        }
        return false;
	}
	
	TokenRuleResult doListPrefixRule(const TokenRuleContext& context){
        std::size_t pos = context.pagePos;
		ListPrefix prefix;
		prefix.degree = 1;
		while(pos < context.page.size()){
			if(context.page[pos] == ' '){
                prefix.degree++;
            }
            else if(check(context.page, pos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
                pos++;//the unicode space is 2 bytes long so we need to add in an extra increment
                prefix.degree++;
            }
			else if(check(context.page, pos, "* ")){
				prefix.type = ListPrefix::Type::Bullet;
				pos += 2;//skip the "* "
				break;
			}
			else if(check(context.page, pos, "# ")){
                prefix.type = ListPrefix::Type::Number;
				pos += 2;//skip the "# "
				break;
			}
			pos++;
		}
		
		TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{prefix, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
	
	void toHtmlNodeList(const HtmlContext& con, const Node& nod){
        const List& node = std::get<List>(nod.node);
        
        switch(node.type){
            case List::Type::Bullet:
                con.out << "<ul>"_AM;
                delegateNodeBranches(con, nod);
                con.out << "</ul>"_AM;
                break;
            case List::Type::Number:
                con.out << "<ol>"_AM;
                delegateNodeBranches(con, nod);
                con.out << "</ol>"_AM;
                break;
            default:
                throw std::runtime_error("Invalid List type");
        }
    }
	
	void toHtmlNodeListElement(const HtmlContext& con, const Node& nod){
        const ListElement& node = std::get<ListElement>(nod.node);
        con.out << "<li>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</li>"_AM;
	}
	
    void handleAdvList(TreeContext& context, const Token& token){
		handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeDivPushable(context);
            if(check(section.typeString, 0, "ul")){
				pushStack(context, Node{AdvList{AdvList::Type::Bullet, section.parameters}});
			}
			else if(check(section.typeString, 0, "ol")){
				pushStack(context, Node{AdvList{AdvList::Type::Number, section.parameters}});
			}
			else{
				throw std::runtime_error("Invalid List type");
			}
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::AdvList)){
                popSingle(context, Node::Type::AdvList);
            }
        });
    }
    
    void handleAdvListElement(TreeContext& context, const Token& token){
		handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            if(isInside(context, Node::Type::AdvList)){
                makeTop(context, Node::Type::AdvList);
                pushStack(context, Node{AdvListElement{section.parameters}});
            }
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::AdvListElement)){
                popSingle(context, Node::Type::AdvListElement);
            }
        });
    }
	
	void toHtmlNodeAdvList(const HtmlContext& con, const Node& nod){
        const AdvList& node = std::get<AdvList>(nod.node);
        switch(node.type){
            case List::Type::Bullet:
                con.out << "<ul"_AM;
                break;
            case List::Type::Number:
                con.out << "<ol"_AM;
                break;
            default:
                throw std::runtime_error("Invalid List type");
        }
        for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << ">"_AM;
        delegateNodeBranches(con, nod);
        switch(node.type){
            case List::Type::Bullet:
                con.out << "</ul>"_AM;
                break;
            case List::Type::Number:
                con.out << "</ol>"_AM;
                break;
            default:
                throw std::runtime_error("Invalid List type");
        }
	}
	
	void toHtmlNodeAdvListElement(const HtmlContext& con, const Node& nod){
        const AdvListElement& node = std::get<AdvListElement>(nod.node);
        con.out << "<li"_AM;
        for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << ">"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</li>"_AM;
	}
}
