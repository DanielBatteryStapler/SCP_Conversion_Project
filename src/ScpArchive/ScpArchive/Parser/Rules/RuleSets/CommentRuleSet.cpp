#include "CommentRuleSet.hpp"

namespace Parser{
    bool tryCommentRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "[!--")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doCommentRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos;
		bool literal = false;
		while(pos < context.page.size()){
			if(!literal && check(context.page, pos, "--]")){
				pos += 3;
				break;
			}
			if(!literal && check(context.page, pos, "@@") && checkParagraph(context.page, pos + 2, "@@")){
				literal = true;
				pos += 2;
			}
			if(literal && check(context.page, pos, "@@")){
				literal = false;
				pos += 2;
			}
			pos++;
		}
		
		TokenRuleResult result;
		result.newPos = pos;
		result.nowNewline = context.wasNewLine;//preserve newline status
		return result;
	}
}
