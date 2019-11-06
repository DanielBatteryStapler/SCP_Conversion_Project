#include "TokenRules.hpp"

#include "../HTTP/HtmlEntity.hpp"

namespace Parser{
	namespace{
		bool check(const std::string& buffer, std::size_t pos, std::string text){
			if(pos + text.size() > buffer.size()){
				return false;
			}
			std::string temp = buffer.substr(pos, text.size());
			return text == temp;
		}
		
		bool checkLine(const std::string& buffer, std::size_t pos, std::string text){
            while(pos < buffer.size()){
                if(check(buffer, pos, text)){
                    return true;
                }
                if(check(buffer, pos, "\n")){
                    return false;
                }
                pos++;
            }
            return false;
		}
		
		bool checkParagraph(const std::string& buffer, std::size_t pos, std::string text){
			while(pos < buffer.size()){
                if(check(buffer, pos, text)){
                    return true;
                }
                if(check(buffer, pos, "\n\n")){
                    return false;
                }
                pos++;
            }
            return false;
		}
		
		bool checkParagraphBack(const std::string& buffer, std::size_t pos, std::string text){
			while(pos >= 0){
                if(check(buffer, pos, text)){
                    return true;
                }
                if(check(buffer, pos, "\n\n")){
                    return false;
                }
                pos--;
            }
            return false;
		}
	}
	
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
			if(!literal && check(context.page, pos, "@@") && checkLine(context.page, pos + 2, "@@")){
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
		return result;
	}
	
	bool tryEntityEscapeRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "@<") && checkLine(context.page, context.pagePos + 2, ">@")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doEntityEscapeRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos + 2;
		std::size_t startPos = pos;
		std::size_t endPos;
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
		std::size_t endPos;
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
	
	bool tryLineBreakRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, " _\n")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doLineBreakRule(const TokenRuleContext& context){
		TokenRuleResult result;
		
		result.newPos = context.pagePos + 3;
		result.newTokens.push_back(Token{LineBreak{}, context.pagePos, context.pagePos + 3, context.page.substr(context.pagePos, 3)});
		result.nowNewline = true;
		
		return result;
	}
	
	bool tryEscapedNewLineRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "\\\n")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doEscapedNewLineRule(const TokenRuleContext& context){
		TokenRuleResult result;
		
		result.newPos = context.pagePos + 2;
		
		return result;
	}
	
	bool tryNewLineRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "\n")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doNewLineRule(const TokenRuleContext& context){
		TokenRuleResult result;
		
		result.nowNewline = true;
		result.newTokens.push_back(Token{NewLine{}, context.pagePos, context.pagePos + 1, context.page.substr(context.pagePos, 1)});
		result.newPos = context.pagePos + 1;
		
		return result;
	}
	
	bool tryTypographyRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "``") && checkParagraph(context.page, context.pagePos, "''")){
			return true;
		}
		if(check(context.page, context.pagePos, "''") && checkParagraphBack(context.page, context.pagePos, "``")){
			return true;
		}
		
		if(check(context.page, context.pagePos, "`") && checkParagraph(context.page, context.pagePos, "'")){
			return true;
		}
		if(check(context.page, context.pagePos, "'") && checkParagraphBack(context.page, context.pagePos, "`")){
			return true;
		}
		
		if(check(context.page, context.pagePos, ",,") && checkParagraph(context.page, context.pagePos, "''")){
			return true;
		}
		if(check(context.page, context.pagePos, "''") && checkParagraphBack(context.page, context.pagePos, ",,")){
			return true;
		}
		
		if(check(context.page, context.pagePos, "--")){
			return true;
		}
		if(check(context.page, context.pagePos, "<<")){
			return true;
		}
		if(check(context.page, context.pagePos, ">>")){
			return true;
		}
		if(check(context.page, context.pagePos, "...")){
			return true;
		}
		
		return false;
	}
	
	TokenRuleResult doTypographyRule(const TokenRuleContext& context){
		std::size_t size;
		std::string output;
		
		if(check(context.page, context.pagePos, "``")){
			size = 2;
			output = "“";
		}
		else if(check(context.page, context.pagePos, "''")){
			size = 2;
			output = "”";
		}
		else if(check(context.page, context.pagePos, "`")){
			size = 1;
			output = "‘";
		}
		else if(check(context.page, context.pagePos, "'")){
			size = 1;
			output = "’";
		}
		else if(check(context.page, context.pagePos, ",,")){
			size = 2;
			output = "„";
		}
		else if(check(context.page, context.pagePos, "--")){
			size = 2;
			output = "—";
		}
		else if(check(context.page, context.pagePos, "<<")){
			size = 2;
			output = "«";
		}
		else if(check(context.page, context.pagePos, ">>")){
			size = 2;
			output = "»";
		}
		else if(check(context.page, context.pagePos, "...")){
			size = 3;
			output = "…";
		}
		
		TokenRuleResult result;
		result.newPos = context.pagePos + size;
		result.newTokens.push_back(Token{PlainText{output}, context.pagePos, context.pagePos + size, context.page.substr(context.pagePos, size)});
		return result;
	}
	
	bool tryPlainTextRule(const TokenRuleContext& context){
		return true;
	}
	
	TokenRuleResult doPlainTextRule(const TokenRuleContext& context){
		TokenRuleResult result;
		
		result.newTokens.push_back(Token{PlainText{context.page[context.pagePos] + std::string("")}, context.pagePos, context.pagePos + 1, context.page[context.pagePos] + std::string("")});
		result.newPos = context.pagePos + 1;
		
		return result;
	}
}
