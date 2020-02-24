#include "QuoteBoxRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenQuoteBoxPrefix(const TokenVariant& tok){
        const QuoteBoxPrefix& quoteBoxPrefix = std::get<QuoteBoxPrefix>(tok);
        return quoteBoxPrefix.degree;
    }
    
    nlohmann::json printNodeQuoteBox(const NodeVariant& nod){
        return {};
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
	
	void toHtmlNodeQuoteBox(const HtmlContext& con, const Node& nod){
        const QuoteBox& node = std::get<QuoteBox>(nod.node);
        con.out << "<blockquote>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</blockquote>"_AM;
	}
}
