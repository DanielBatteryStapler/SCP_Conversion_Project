#include "Parser.hpp"

#include "TokenRules.hpp"

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
	
	bool SectionStart::operator==(const SectionStart& tok)const{
		return type == tok.type && moduleType == tok.moduleType && parameters == tok.parameters;
	}
		
	bool SectionEnd::operator==(const SectionEnd& tok)const{
		return type == tok.type;
	}
	
	bool Divider::operator==(const Divider& tok)const{
		return type == tok.type;
	}
		
	bool PrefixFormat::operator==(const PrefixFormat& tok)const{
		return type == tok.type;
	}
		
	bool NestingPrefixFormat::operator==(const NestingPrefixFormat& tok)const{
		return type == tok.type && degree == tok.degree;
	}
		
	bool InlineFormat::operator==(const InlineFormat& tok)const{
		return type == tok.type;
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
	
	std::string toString(const Token& tok){
		
		std::stringstream ss;
		
		ss << "{";
		switch(tok.getType()){
			default:
				ss << "Unknown";
				break;
			case Token::Type::SectionStart:
				ss << "SectionStart";
				break;
			case Token::Type::SectionEnd:
				ss << "SectionEnd";
				break;
			case Token::Type::SectionComplete:
				ss << "SectionComplete";
				break;
			case Token::Type::Divider:
				ss << "Divider";
				break;
			case Token::Type::PrefixFormat:
				ss << "PrefixFormat";
				break;
			case Token::Type::NestingPrefixFormat:
				ss << "NestingPrefixFormat";
				break;
			case Token::Type::InlineSectionStart:
				ss << "InlineSectionStart";
				break;
			case Token::Type::InlineSectionEnd:
				ss << "InlineSectionEnd";
				break;
			case Token::Type::InlineFormat:
				ss << "InlineFormat";
				break;
			case Token::Type::HyperLink:
				{
					const HyperLink& link = std::get<HyperLink>(tok.token);
					ss << "HyperLink:\"" << link.shownText << "\", \"" << link.url << "\", " << (link.newWindow?"true":"false");
				}
				break;
			case Token::Type::LiteralText:
				ss << "LiteralText:\"" << std::get<LiteralText>(tok.token).text << "\"";
				break;
			case Token::Type::PlainText:
				ss << "PlainText:\"" << std::get<PlainText>(tok.token).text << "\"";
				break;
			case Token::Type::LineBreak:
				ss << "LineBreak";
				break;
			case Token::Type::NewLine:
				ss << "NewLine";
				break;
		}
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
	
	TokenedPage tokenizePage(std::string page){
		
		std::vector<TokenRule> rules = {
			TokenRule{"comment", tryCommentRule, doCommentRule},
			TokenRule{"tripleLink", tryTripleLinkRule, doTripleLinkRule},
			TokenRule{"singleLink", trySingleLinkRule, doSingleLinkRule},
			TokenRule{"bareLink", tryBareLinkRule, doBareLinkRule},
			TokenRule{"entityEscape", tryEntityEscapeRule, doEntityEscapeRule},
			TokenRule{"literalText", tryLiteralTextRule, doLiteralTextRule}, 
			TokenRule{"lineBreak", tryLineBreakRule, doLineBreakRule},
			TokenRule{"escapedNewLine", tryEscapedNewLineRule, doEscapedNewLineRule},
			TokenRule{"newLine", tryNewLineRule, doNewLineRule},
			TokenRule{"typography", tryTypographyRule, doTypographyRule},
			TokenRule{"plainText", tryPlainTextRule, doPlainTextRule}
		};
		
		TokenRuleContext context;
		context.page = std::move(page);
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
		
		TokenedPage output;
		
		output.originalPage = std::move(context.page);
		//output.tokens = context.tokens;
		
		///*
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
		//*/
		
		return output;
	}
}
