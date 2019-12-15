#include "HeadingRuleSet.hpp"

namespace Parser{
    std::string toStringTokenHeading(const TokenVariant& tok){
        const Heading& heading = std::get<Heading>(tok);
        return "Heading:" + heading.degree + std::string{", "} + (heading.hidden?"true":"false");
    }
    
    std::string toStringNodeHeading(const NodeVariant& nod){
        return toStringTokenHeading(std::get<Heading>(nod));
    }
	
	bool tryHeadingRule(const TokenRuleContext& context){
        if(context.wasNewLine && check(context.page, context.pagePos, "+")){
            std::size_t pos = context.pagePos;
            while(pos < context.page.size()){
                if(check(context.page, pos, " ") || check(context.page, pos, "* ")){
                    return true;
                }
                else if(context.page[pos] != '+'){
                    return false;
                }
                pos++;
            }
            return false;
        }
        return false;
	}
	
	TokenRuleResult doHeadingRule(const TokenRuleContext& context){
        Heading heading;
        heading.degree = 0;
        
        std::size_t pos = context.pagePos;
        while(pos < context.page.size()){
            if(check(context.page, pos, " ")){
                pos++;
                heading.hidden = false;
                break;
            }
            else if(check(context.page, pos, "* ")){
                pos += 2;
                heading.hidden = true;
                break;
            }
            pos++;
            heading.degree++;
        }
        
        TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{heading, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
	
    void handleHeading(TreeContext& context, const Token& token){
        makeDivPushable(context);
        pushStack(context, Node{std::get<Heading>(token.token)});
    }
}
