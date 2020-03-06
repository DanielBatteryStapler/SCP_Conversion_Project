#include "EntityEscapeRuleSet.hpp"

#include "../../../HTTP/HtmlEntity.hpp"

namespace Parser{
	bool tryEntityEscapeRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "@<") && checkLine(context.page, context.pagePos + 2, ">@")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doEntityEscapeRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos + 2;
		std::size_t startPos = pos;
		std::size_t endPos = 0;
		while(pos < context.page.size()){
			if(check(context.page, pos, ">@")){
				endPos = pos;
				pos += 2;
				break;
			}
			pos++;
		}
		
		std::string rawSource = context.page.substr(context.pagePos, pos - context.pagePos);
		std::string content = decodeHtmlEntities(context.page.substr(startPos, endPos - startPos));
		
		TokenRuleResult result;
		result.newPos = pos;
		result.newTokens.push_back(Token{LiteralText{content}, context.pagePos, pos, rawSource});
		return result;
	}
}
