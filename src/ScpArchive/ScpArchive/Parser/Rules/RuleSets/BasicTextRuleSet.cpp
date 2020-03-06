#include "BasicTextRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenPlainText(const TokenVariant& tok){
        return std::get<PlainText>(tok).text;
    }
    
    nlohmann::json printTokenNewLine(const TokenVariant& tok){
        return {};
    }
    
    nlohmann::json printTokenLineBreak(const TokenVariant& tok){
        return {};
    }
    
    nlohmann::json printNodePlainText(const NodeVariant& nod){
        return std::get<PlainText>(nod).text;
    }
    
    nlohmann::json printNodeParagraph(const NodeVariant& nod){
        return {};
    }
    
    nlohmann::json printNodeLineBreak(const NodeVariant& nod){
        return {};
    }
    
    nlohmann::json printNodeRootPage(const NodeVariant& nod){
        return {};
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
		else{
			throw std::runtime_error("Error when processing typography token");
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
	
	void handlePlainText(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<PlainText>(token.token)});
    }
    
    void handleLineBreak(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<LineBreak>(token.token)});
    }
    
    void toHtmlNodeRootPage(const HtmlContext& con, const Node& nod){
        delegateNodeBranches(con, nod);
    }
    
	void toHtmlNodeParagraph(const HtmlContext& con, const Node& nod){
        con.out << "<p>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</p>"_AM;
	}
	
	void toHtmlNodeLineBreak(const HtmlContext& con, const Node& nod){
        con.out << "<br />"_AM;
	}
	
	void toHtmlNodePlainText(const HtmlContext& con, const Node& nod){
        const PlainText& node = std::get<PlainText>(nod.node);
        con.out << node.text;
	}
}
