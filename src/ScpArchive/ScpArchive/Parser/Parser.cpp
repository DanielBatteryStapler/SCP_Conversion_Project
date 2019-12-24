#include "Parser.hpp"

#include "Rules/RuleSet.hpp"

#include <sstream>

namespace Parser{
	std::string& trimLeft(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}
	// trim from end
	std::string& trimRight(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}
	// trim from both ends
	std::string& trimString(std::string& s) {
		return trimLeft(trimRight(s));
	}
	
	std::string normalizePageName(std::string link){
		std::string output;
		for(char c : link){
			if(isalnum(c) || c == '#'){
				output += tolower(c);
			}
			else if(c == ':'){
				while(output.size() > 0 && output.back() == '-'){//remove trailing white space
					output.erase(output.size() - 1);
				}
				output += ':';
			}
			else if(isspace(c) || c == '-' || c == '.'){
				if(output.size() == 0 || output.back() == ':'){
					//do nothing
				}
				else{
                    output += '-';
				}
			}
		}
		while(output.size() > 0 && output.back() == '-'){//remove trailing white space
			output.erase(output.size() - 1);
		}
		return output;
	}
	
	bool Section::operator==(const Section& tok)const{
		return type == tok.type && typeString == tok.typeString && moduleType == tok.moduleType && mainParameter == tok.mainParameter && parameters == tok.parameters;
	}
		
	bool SectionEnd::operator==(const SectionEnd& tok)const{
		return type == tok.type && typeString == tok.typeString;
	}
	
	bool SectionComplete::operator==(const SectionComplete& tok)const{
		return type == tok.type && typeString == tok.typeString && moduleType == tok.moduleType && mainParameter == tok.mainParameter && parameters == tok.parameters && contents == tok.contents;
	}
	
	bool Divider::operator==(const Divider& tok)const{
		return type == tok.type;
	}
    
	bool Heading::operator==(const Heading& tok)const{
		return degree == tok.degree && hidden == tok.hidden;
	}
		
	bool QuoteBoxPrefix::operator==(const QuoteBoxPrefix& tok)const{
		return degree == tok.degree;
	}
	
    bool ListPrefix::operator==(const ListPrefix& tok)const{
        return type == tok.type && degree == tok.degree;
    }
	
	bool InlineFormat::operator==(const InlineFormat& tok)const{
		return type == tok.type && begin == tok.begin && end == tok.end && color == tok.color;
	}
	
	bool TableMarker::operator==(const TableMarker& tok)const{
        return type == tok.type && alignment == tok.alignment && span == tok.span;
	}
	
	bool HyperLink::operator==(const HyperLink& tok)const{
		return shownText == tok.shownText && url == tok.url && newWindow == tok.newWindow;
	}
	
	bool LiteralText::operator==(const LiteralText& tok)const{
		return text == tok.text;
	}
		
	bool PlainText::operator==(const PlainText& tok)const{
		return text == tok.text;
	}
	
	bool CenterText::operator==(const CenterText& tok)const{
        return true;
	}
	
	bool LineBreak::operator==(const LineBreak& tok)const{
		return true;
	}
	
	bool NewLine::operator==(const NewLine& tok)const{
		return true;
	}
	
	Token::Type Token::getType()const{
		return static_cast<Type>(token.index());
	}
	
	bool Token::operator==(const Token& tok)const{
		return token == tok.token && sourceStart == tok.sourceStart && sourceEnd == tok.sourceEnd && source == tok.source;
	}
	
	std::string tokenVariantToString(const Token& tok){
		std::stringstream ss;
		
		const std::vector<TokenPrintRule> tokenPrintRules = getTokenPrintRules();
		Token::Type tokType = tok.getType();
		
		for(const TokenPrintRule& printRule : tokenPrintRules){
            if(printRule.type == tokType){
                return printRule.toString(tok.token);
            }
		}
		throw std::runtime_error("Attempted to print a Token with no valid TokenPrintRule");
	}
	
	std::string toString(const Token& tok){
		
		std::stringstream ss;
		
		ss << "{";
		ss << tokenVariantToString(tok);
		ss << "} -> [" << tok.sourceStart << ", " << tok.sourceEnd << ") = \"";
		for(char c : tok.source){
			switch(c){
				default:
					ss << c;
					break;
				case '\n':
					ss << "\\n";
					break;
				case '\r':
					ss << "\\r";
					break;
				case '\t':
					ss << "\\t";
					break;
			}
		}
		ss << "\"";
		return ss.str();
	}
	
	std::ostream& operator<<(std::ostream& out, const Token& tok){
		out << toString(tok);
		return out;
	}
	
	namespace{
		TokenRuleContext applyTokenizingRules(std::string&& page, std::vector<TokenRule> rules){
			TokenRuleContext context;
			context.page = page;
			context.pagePos = 0;
			context.wasNewLine = true;
			for(; context.pagePos < context.page.size();){
				bool allRulesFailed = true;
				for(const TokenRule& rule : rules){
					if(rule.tryRule(context)){
						allRulesFailed = false;
						
						TokenRuleResult result = rule.doRule(context);
						context.pagePos = result.newPos;
						context.wasNewLine = result.nowNewline;
						context.tokens.insert(context.tokens.end(), result.newTokens.begin(), result.newTokens.end());
						
						break;
					}
				}
				if(allRulesFailed){
					throw std::runtime_error("All parsing rules failed, this should not be possible");
				}
			}
			return context;
		}
	}
	
	TokenedPage tokenizePage(std::string page, ParserParameters parameters){
		//just to save myself some headaches, lets replace all nonbreaking spaces with normal spaces
		//this might be refactored out in the future if the token rules are updated to deal with nonbreaking spaces more correctly
		const auto replaceAll = [](std::string str, const std::string& from, const std::string& to)->std::string{
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        };
		
		page = replaceAll(page, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)}, " ");
		
		//for the include parameters, we're just gonna replaceAll on them to make it easy
		for(auto i = parameters.includeParameters.begin(); i != parameters.includeParameters.end(); i++){
            page = replaceAll(page, "{$" + i->first + "}", i->second);
		}
		
		TokenRuleContext context = applyTokenizingRules(std::move(page), getTokenRules());
		
		TokenedPage output;
		output.originalPage = std::move(context.page);
		
		//merge all PlainText Tokens
		for(const Token& tok : context.tokens){
			if(output.tokens.size() > 0){
				Token& back = output.tokens.back();
				if(back.getType() == Token::Type::PlainText && tok.getType() == Token::Type::PlainText
						&& back.sourceEnd == tok.sourceStart){
					PlainText& backText = std::get<PlainText>(back.token);
					const PlainText& tokText = std::get<PlainText>(tok.token);
					backText.text += tokText.text;
					back.source += tok.source;
					back.sourceEnd = tok.sourceEnd;
					
					continue;
				}
			}
			output.tokens.push_back(tok);
		}
		
		return output;
	}
	
	std::vector<std::string> getPageLinks(std::string page){
		const std::vector<TokenRule> rules = getTokenRules();
		/*{
			TokenRule{tryCommentRule, doCommentRule},
			TokenRule{tryTripleLinkRule, doTripleLinkRule},
			TokenRule{trySingleLinkRule, doSingleLinkRule},
			TokenRule{tryNullRule, doNullRule}
		};*/
		
		TokenRuleContext context = applyTokenizingRules(std::move(page), rules);
		std::vector<std::string> links;
		
		for(const Token& tok : context.tokens){
			if(tok.getType() == Token::Type::HyperLink){
				HyperLink link = std::get<HyperLink>(tok.token);
				if(!check(link.url, 0, "http://") && !check(link.url, 0, "https://")){
					links.push_back(link.url);
				}
			}
		}
		
		return links;
	}
}










