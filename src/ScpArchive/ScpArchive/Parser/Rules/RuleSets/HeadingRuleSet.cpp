#include "HeadingRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenHeading(const TokenVariant& tok){
        const Heading& heading = std::get<Heading>(tok);
        nlohmann::json out;
        out["degree"] = heading.degree;
        out["hidden"] = heading.hidden;
        return out;
    }
    
    nlohmann::json printNodeHeading(const NodeVariant& nod){
        return printTokenHeading(std::get<Heading>(nod));
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
    
	void toHtmlNodeHeading(const HtmlContext& con, const Node& nod){
        const Heading& node = std::get<Heading>(nod.node);
        con.out << "<h"_AM << std::to_string(node.degree) << " id='toc"_AM << std::to_string(node.tocNumber) << "'>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</h"_AM << std::to_string(node.degree) << ">"_AM;
	}
}
