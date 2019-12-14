#include "HyperLinkRuleSet.hpp"

namespace Parser{
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
	
    void handleHyperLink(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<HyperLink>(token.token)});
    }
}
