#include "TokenRules.hpp"

#include "../HTTP/HtmlEntity.hpp"

namespace Parser{
	namespace{
		inline bool check(const std::string& buffer, std::size_t pos, std::string text){
			if(pos + text.size() > buffer.size()){
				return false;
			}
			std::string temp = buffer.substr(pos, text.size());
			return text == temp;
		}
		
		inline bool checkLine(const std::string& buffer, std::size_t pos, std::string text){
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
		
		inline bool checkParagraph(const std::string& buffer, std::size_t pos, std::string text){
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
		
		inline bool checkParagraphBack(const std::string& buffer, std::size_t pos, std::string text){
			while(true){
                if(check(buffer, pos, text)){
                    return true;
                }
                if(check(buffer, pos, "\n\n")){
                    return false;
                }
                if(pos == 0){
					break;
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
		return result;
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
	
	bool tryTripleLinkRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "[[[") && checkParagraph(context.page, context.pagePos, "]]]")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doTripleLinkRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos + 3;//skip the first bracket
		
		HyperLink link;
		if(check(context.page, pos, "*https://") || check(context.page, pos, "*http://")){
			link.newWindow = true;
			pos++;
		}
		else{
			link.newWindow = false;
		}
		
		while(pos < context.page.size()){
			if(check(context.page, pos, "]]]") || check(context.page, pos, "|")){
				break;
			}
			link.url += context.page[pos];
			pos++;
		}
		bool hasPipe = check(context.page, pos, "|");
		if(hasPipe){
			pos++;//skip over the pipe
		}
		while(pos < context.page.size()){
			if(check(context.page, pos, "]]]")){
				break;
			}
			link.shownText += context.page[pos];
			pos++;
		}
		pos += 3;//move it past the last bracket
		trimString(link.shownText);
		trimString(link.url);
		
		if(link.shownText == ""){
			link.shownText = link.url;
			if(!check(link.url, 0, "https://") && !check(link.url, 0, "http://")){
				link.url = normalizePageName(link.url);
			}
			if(hasPipe){
				link.shownText = link.url;
			}
		}
		else{
			if(!check(link.url, 0, "https://") && !check(link.url, 0, "http://")){
				link.url = normalizePageName(link.url);
			}
		}
		
		TokenRuleResult result;
		result.newPos = pos;
		result.newTokens.push_back(Token{link, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
		return result;
	}
	
	bool trySingleLinkRule(const TokenRuleContext& context){
		if(checkLine(context.page, context.pagePos, "]")){
			if(check(context.page, context.pagePos, "[https://")){
				return true;
			}
			if(check(context.page, context.pagePos, "[http://")){
				return true;
			}
			if(check(context.page, context.pagePos, "[*https://")){
				return true;
			}
			if(check(context.page, context.pagePos, "[*http://")){
				return true;
			}
		}
		return false;
	}
	
	TokenRuleResult doSingleLinkRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos + 1;//skip the first bracket
		
		HyperLink link;
		if(check(context.page, pos, "*")){
			link.newWindow = true;
			pos++;
		}
		else{
			link.newWindow = false;
		}
		
		while(pos < context.page.size()){
			char c = context.page[pos];
			if(isspace(c) || c == ']'){
				break;
			}
			link.url += c;
			pos++;
		}
		while(pos < context.page.size()){//skip white space in the middle
			char c = context.page[pos];
			if(!isspace(c)){
				break;
			}
			pos++;
		}
		while(pos < context.page.size()){
			char c = context.page[pos];
			if(c == ']'){
				break;
			}
			link.shownText += c;
			pos++;
		}
		pos++;//move it past the last bracket
		if(link.shownText == ""){
			link.shownText = link.url;
		}
		
		TokenRuleResult result;
		result.newPos = pos;
		result.newTokens.push_back(Token{link, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
		return result;
	}
	
	bool tryBareLinkRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "https://")){
			return true;
		}
		if(check(context.page, context.pagePos, "http://")){
			return true;
		}
		if(check(context.page, context.pagePos, "*https://")){
			return true;
		}
		if(check(context.page, context.pagePos, "*http://")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doBareLinkRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos;
		
		HyperLink link;
		if(check(context.page, pos, "*")){
			link.newWindow = true;
			pos++;
		}
		else{
			link.newWindow = false;
		}
		
		while(pos < context.page.size()){
			char c = context.page[pos];
			if(isspace(c)){
				break;
			}
			link.url += c;
			pos++;
		}
		link.shownText = link.url;
		
		TokenRuleResult result;
		result.newPos = pos;
		result.newTokens.push_back(Token{link, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
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
