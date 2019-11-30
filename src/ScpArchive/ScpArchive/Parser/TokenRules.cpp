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
	
	bool tryCarriageReturn(const TokenRuleContext& context){
        return check(context.page, context.pagePos, "\r");
	}
	
	TokenRuleResult doCarriageReturn(const TokenRuleContext& context){
        TokenRuleResult result;
        result.newPos = context.pagePos + 1;
        result.nowNewline = context.wasNewLine;//make sure to preserve newline status
        return result;
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
		result.nowNewline = context.wasNewLine;//preserve newline status
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
	
	bool tryDividerRule(const TokenRuleContext& context){
		if(context.wasNewLine){
			const auto checkFunction = [&context](char c)->bool{
				std::size_t pos = context.pagePos;
				while(pos < context.page.size()){
					if(context.page[pos] == '\n'){
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
			divider.type = Divider::Type::Clear;
		}
		
		std::size_t pos = context.pagePos;
		while(pos < context.page.size()){
			if(context.page[pos] == '\n'){
				break;
			}
			pos++;
		}
		
		TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{divider, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
	
	bool tryQuoteBoxPrefixRule(const TokenRuleContext& context){
		if(context.tokens.size() > 0 && context.tokens.back().getType() == Token::Type::QuoteBoxPrefix){
			return false;//a quote box prefix cannot directly follow another quote box
		}
		if(context.wasNewLine && check(context.page, context.pagePos, ">")){
			std::size_t pos = context.pagePos;
			while(pos < context.page.size()){
				if(check(context.page, pos, ">")){
					//keep going
				}
				else if(check(context.page, pos, " ") || check(context.page, pos, "\n")){
					return true;
				}
				else{
					return false;//fail if the quote box prefix ends with anything other than a space
				}
				pos++;
			}
		}
		return false;
	}
	
	TokenRuleResult doQuoteBoxPrefixRule(const TokenRuleContext& context){
		std::size_t pos = context.pagePos;
		QuoteBoxPrefix prefix;
		while(pos < context.page.size()){
			if(check(context.page, pos, " ")){
				prefix.degree = pos - context.pagePos;
				pos++;//skip the space too
				break;
			}
			else if(check(context.page, pos, "\n")){
				prefix.degree = pos - context.pagePos;
				break;
			}
			pos++;
		}
		
		
		TokenRuleResult result;
        result.newPos = pos;
        result.nowNewline = true;//preserve newline status
        result.newTokens.push_back(Token{prefix, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
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
	
	namespace{
		inline bool handleTryInlineFormatting(const TokenRuleContext& context, std::string format){
			if(!check(context.page, context.pagePos, format)){
				return false;
			}
			if(context.page.size() <= context.pagePos + format.size() || isspace(context.page[context.pagePos + format.size()])){
				if(0 > context.pagePos - 1 || isspace(context.page[context.pagePos - 1])){
					return false;
				}
			}
			return true;
		}
		
		inline TokenRuleResult handleDoInlineFormatting(const TokenRuleContext& context, InlineFormat::Type type, std::string format){
			
			InlineFormat token;
			token.type = type;
			if(context.page.size() <= context.pagePos + format.size() || isspace(context.page[context.pagePos + format.size()])){
				token.begin = false;
			}
			else{
				token.begin = true;
			}
			
			if(0 > context.pagePos - 1 || isspace(context.page[context.pagePos - 1])){
				token.end = false;
			}
			else{
				token.end = true;
			}
			
			const std::size_t begin = context.pagePos;
			const std::size_t end = context.pagePos + format.size();
			
			TokenRuleResult result;
			result.newPos = end;
			result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
			return result;
		}
	}
	
	bool tryStrikeRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "--");
	}
	TokenRuleResult doStrikeRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Strike, "--");
	}
	
	bool tryItalicsRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "//");
	}
	TokenRuleResult doItalicsRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Italics, "//");
	}
	
	bool tryBoldRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "**");
	}
	TokenRuleResult doBoldRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Bold, "**");
	}
	
	bool tryUnderlineRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "__");
	}
	TokenRuleResult doUnderlineRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Underline, "__");
	}
	
	bool trySuperRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "^^");
	}
	TokenRuleResult doSuperRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Super, "^^");
	}
	
	bool trySubRule(const TokenRuleContext& context){
		return !checkParagraph(context.page, context.pagePos, "''") && handleTryInlineFormatting(context, ",,");
	}
	TokenRuleResult doSubRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Sub, ",,");
	}
	
	bool tryMonospaceRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "{{")){
			return true;
		}
		else if(check(context.page, context.pagePos, "}}")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doMonospaceRule(const TokenRuleContext& context){
		InlineFormat token;
		token.type = InlineFormat::Type::Monospace;
		if(check(context.page, context.pagePos, "{{")){
			token.begin = true;
			token.end = false;
		}
		else{
			token.begin = false;
			token.end = true;
		}
		
		const std::size_t begin = context.pagePos;
		const std::size_t end = context.pagePos + 2;
		
		TokenRuleResult result;
		result.newPos = end;
		result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
		return result;
	}
	
	bool tryColorRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "##")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doColorRule(const TokenRuleContext& context){
		const std::size_t begin = context.pagePos;
		std::size_t end = context.pagePos;
		
		InlineFormat token;
		token.type = InlineFormat::Type::Color;
		if(checkLine(context.page, context.pagePos, "|")){
			token.begin = true;
			token.end = false;
			end += 2;
			while(context.page[end] != '|'){
				end++;
			}
			token.color = context.page.substr(begin + 2, end - begin - 2);
			token.color.erase(remove(token.color.begin(),token.color.end(),' '),token.color.end());//remove all whitespace
			end++;
		}
		else{
			end += 2;
			token.begin = false;
			token.end = true;
		}
		
		TokenRuleResult result;
		result.newPos = end;
		result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
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
			if(check(context.page, context.pagePos, "[#")){
				return true;
			}
			if(check(context.page, context.pagePos, "[*https://")){
				return true;
			}
			if(check(context.page, context.pagePos, "[*http://")){
				return true;
			}
			if(check(context.page, context.pagePos, "[*#")){
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
			if(isalnum(c) || c == '.' || c == '-' || c == ':' || c == '/' || c == '_' || c == '&' || c == '?' || c == '#' || c == '=' || c == '%'){
				link.url += c;
				pos++;
			}
			else{//if you see a character that can't be in a url, then the url is finished
				break;
			}
			
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
	
	bool tryPrelineSpaceRule(const TokenRuleContext& context){
		if(context.wasNewLine){
			if(context.page[context.pagePos] == ' ' || check(context.page, context.pagePos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
				return true;
			}
		}
		return false;
	}
	
	TokenRuleResult doPrelineSpaceRule(const TokenRuleContext& context){
		TokenRuleResult result;
		if(check(context.page, context.pagePos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
			result.newPos = context.pagePos + 2;
		}
		else{
			result.newPos = context.pagePos + 1;
		}
		result.nowNewline = true;//act as if we're still past a newline
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
	
	bool tryNullRule(const TokenRuleContext& context){
		return true;
	}
	
	TokenRuleResult doNullRule(const TokenRuleContext& context){
		TokenRuleResult result;
		result.newPos = context.pagePos + 1;
		return result;
	}
}
