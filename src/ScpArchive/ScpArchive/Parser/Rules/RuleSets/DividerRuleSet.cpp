#include "DividerRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenDivider(const TokenVariant& tok){
        const Divider& divider = std::get<Divider>(tok);
        switch(divider.type){
			default:
				return "Unknown";
			case Divider::Type::Line:
				return "Line";
			case Divider::Type::ClearBoth:
				return "ClearBoth";
			case Divider::Type::ClearLeft:
				return "ClearLeft";
			case Divider::Type::ClearRight:
				return "ClearRight";
			case Divider::Type::Separator:
				return "Separator";
		}
    }
    
    nlohmann::json printNodeDivider(const NodeVariant& nod){
        return printTokenDivider(std::get<Divider>(nod));
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
			else if(check(context.page, context.pagePos, "====")){
				return checkFunction('=');
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
		else if(context.page[context.pagePos] == '='){
			divider.type = Divider::Type::Separator;
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
                con.out << "<div class='PageClearer'></div>"_AM;
                break;
            case Divider::Type::ClearLeft:
                con.out << "<div class='PageClearerLeft'></div>"_AM;
                break;
            case Divider::Type::ClearRight:
                con.out << "<div class='PageClearerRight'></div>"_AM;
                break;
            case Divider::Type::Separator:
                //has no effect on html
                break;
            default:
                throw std::runtime_error("Invalid Divider type");
        }
	}
}
