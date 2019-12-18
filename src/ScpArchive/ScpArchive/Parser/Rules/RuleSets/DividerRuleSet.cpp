#include "DividerRuleSet.hpp"

namespace Parser{
    std::string toStringTokenDivider(const TokenVariant& tok){
        const Divider& divider = std::get<Divider>(tok);
        std::string output = "Divider:";
        switch(divider.type){
        default:
            output += "Unknown";
            break;
        case Divider::Type::Line:
            output += "Line";
            break;
        case Divider::Type::ClearBoth:
            output += "ClearBoth";
            break;
        case Divider::Type::ClearLeft:
            output += "ClearLeft";
            break;
        case Divider::Type::ClearRight:
            output += "ClearRight";
            break;
        }
        return output;
    }
    
    std::string toStringNodeDivider(const NodeVariant& nod){
        return toStringTokenDivider(std::get<Divider>(nod));
    }
   
    bool tryDividerRule(const TokenRuleContext& context){
		if(context.wasNewLine){
			const auto checkFunction = [&context](char c)->bool{
				std::size_t pos = context.pagePos;
				while(pos < context.page.size()){
					if(context.page[pos] == '\n'){
						break;
					}
					else if(c == '~' && (context.page[pos] == '<' || context.page[pos] == '>') && (pos + 1 == context.page.size() || context.page[pos + 1] == '\n')){
                        break;
					}
					else if(context.page[pos] != c){
						return false;
					}
					pos++;
				}
				return true;
			};
			
			if(check(context.page, context.pagePos, "----")){
				return checkFunction('-');
			}
			else if(check(context.page, context.pagePos, "~~~~")){
				return checkFunction('~');
			}
		}
		return false;
	}
	
	TokenRuleResult doDividerRule(const TokenRuleContext& context){
		Divider divider;
		
		if(context.page[context.pagePos] == '-'){
			divider.type = Divider::Type::Line;
		}
		else if(context.page[context.pagePos] == '~'){
			divider.type = Divider::Type::ClearBoth;
		}
		
		std::size_t pos = context.pagePos;
		while(pos < context.page.size()){
			if(context.page[pos] == '\n'){
				break;
			}
			else if(context.page[pos] == '<'){
                divider.type = Divider::Type::ClearLeft;
			}
			else if(context.page[pos] == '>'){
                divider.type = Divider::Type::ClearRight;
			}
			pos++;
		}
		
		TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{divider, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
	
    void handleDivider(TreeContext& context, const Token& token){
        addAsDiv(context, Node{std::get<Divider>(token.token)});
    }
    
	void toHtmlNodeDivider(const HtmlContext& con, const Node& nod){
        const Divider& node = std::get<Divider>(nod.node);
        switch(node.type){
            case Divider::Type::Line:
                con.out << "<hr />"_AM;
                break;
            case Divider::Type::ClearBoth:
                con.out << "<div class='PageClearer' />"_AM;
                break;
            case Divider::Type::ClearLeft:
                con.out << "<div class='PageClearerLeft' />"_AM;
                break;
            case Divider::Type::ClearRight:
                con.out << "<div class='PageClearerRight' />"_AM;
                break;
            default:
                throw std::runtime_error("Invalid Divider type");
        }
	}
}