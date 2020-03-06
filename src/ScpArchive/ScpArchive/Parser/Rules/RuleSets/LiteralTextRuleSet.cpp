#include "LiteralTextRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenLiteralText(const TokenVariant& tok){
        return std::get<LiteralText>(tok).text;
    }
    
    nlohmann::json printNodeLiteralText(const NodeVariant& nod){
        return std::get<LiteralText>(nod).text;
    }
	
	bool tryLiteralTextRule(const TokenRuleContext& context){
        if(check(context.page, context.pagePos, "@@")
                && checkLine(context.page, context.pagePos + 2, "@@")){
            return true;
        }
        return false;
	}
	
	TokenRuleResult doLiteralTextRule(const TokenRuleContext& context){
        std::size_t pos = context.pagePos + 2;
		std::size_t startPos = pos;
		std::size_t endPos = 0;
		while(pos < context.page.size()){
			if(check(context.page, pos, "@@")){
				endPos = pos;
				pos += 2;
				break;
			}
			pos++;
		}
		
		std::string rawSource = context.page.substr(context.pagePos, pos - context.pagePos);
		std::string content = context.page.substr(startPos, endPos - startPos);
		
		TokenRuleResult result;
		result.newPos = pos;
		result.newTokens.push_back(Token{LiteralText{content}, context.pagePos, pos, rawSource});
		return result;
	}
	
    void handleLiteralText(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<LiteralText>(token.token)});
    }
    
	void toHtmlNodeLiteralText(const HtmlContext& con, const Node& nod){
        const LiteralText& node = std::get<LiteralText>(nod.node);
        con.out << node.text;///TODO: this isn't correct, it should have a style that makes some whitespace oddities 
	}
}
