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
                {
					const Divider& divider = std::get<Divider>(tok.token);
                    ss << "Divider:";
                    switch(divider.type){
					default:
						ss << "Unknown";
						break;
					case Divider::Type::Line:
						ss << "Line";
						break;
					case Divider::Type::Clear:
						ss << "Clear";
						break;
                    }
				}
				break;
			case Token::Type::Heading:
                {
					const Heading& heading = std::get<Heading>(tok.token);
                    ss << "Heading:" << heading.degree << ", " << (heading.hidden?"true":"false");
				}
				break;
			case Token::Type::QuoteBoxPrefix:
				{
					const QuoteBoxPrefix& quoteBoxPrefix = std::get<QuoteBoxPrefix>(tok.token);
                    ss << "QuoteBoxPrefix:" << quoteBoxPrefix.degree;
				}
				break;
            case Token::Type::ListPrefix:
				{
					const ListPrefix& listPrefix = std::get<ListPrefix>(tok.token);
                    ss << "ListPrefix:";
                    switch(listPrefix.type){
                    default:
                        ss << "Unknown";
                        break;
                    case ListPrefix::Type::Bullet:
                        ss << "Bullet";
                        break;
                    case ListPrefix::Type::Number:
                        ss << "Number";
                        break;
                    }
                    ss << ", " << listPrefix.degree;
				}
				break;
			case Token::Type::InlineSectionStart:
				ss << "InlineSectionStart";
				break;
			case Token::Type::InlineSectionEnd:
				ss << "InlineSectionEnd";
				break;
			case Token::Type::InlineFormat:
				ss << "InlineFormat:";
				{
					const InlineFormat& format = std::get<InlineFormat>(tok.token);
					switch(format.type){
						default:
							ss << "Unknown";
							break;
						case InlineFormat::Type::Bold:
							ss << "Bold";
							break;
						case InlineFormat::Type::Italics:
							ss << "Italics";
							break;
						case InlineFormat::Type::Strike:
							ss << "Strike";
							break;
						case InlineFormat::Type::Underline:
							ss << "Underline";
							break;
						case InlineFormat::Type::Super:
							ss << "Super";
							break;
						case InlineFormat::Type::Sub:
							ss << "Sub";
							break;
						case InlineFormat::Type::Monospace:
							ss << "Monospace";
							break;
						case InlineFormat::Type::Color:
							ss << "Color";
							break;
					}
					ss << "[" << (format.begin?"true":"false") << "," << (format.end?"true":"false") << "," << format.color << "]";
				}
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
		
		std::vector<TokenRule> standardRules = {
			TokenRule{"comment", tryCommentRule, doCommentRule},
			TokenRule{"heading", tryHeadingRule, doHeadingRule},
			TokenRule{"divider", tryDividerRule, doDividerRule},
			TokenRule{"quoteBoxPrefix", tryQuoteBoxPrefixRule, doQuoteBoxPrefixRule},
			TokenRule{"listPrefix", tryListPrefixRule, doListPrefixRule},
			
			TokenRule{"strike", tryStrikeRule, doStrikeRule},
			TokenRule{"italics", tryItalicsRule, doItalicsRule},
			TokenRule{"bold", tryBoldRule, doBoldRule},
			TokenRule{"underline", tryUnderlineRule, doUnderlineRule},
			TokenRule{"super", trySuperRule, doSuperRule},
			TokenRule{"sub", trySubRule, doSubRule},
			TokenRule{"monospace", tryMonospaceRule, doMonospaceRule},
			TokenRule{"color", tryColorRule, doColorRule},
			
			TokenRule{"tripleLink", tryTripleLinkRule, doTripleLinkRule},
			TokenRule{"singleLink", trySingleLinkRule, doSingleLinkRule},
			TokenRule{"bareLink", tryBareLinkRule, doBareLinkRule},
			
			TokenRule{"entityEscape", tryEntityEscapeRule, doEntityEscapeRule},
			TokenRule{"literalText", tryLiteralTextRule, doLiteralTextRule}, 
			TokenRule{"lineBreak", tryLineBreakRule, doLineBreakRule},
			TokenRule{"escapedNewLine", tryEscapedNewLineRule, doEscapedNewLineRule},
			TokenRule{"newLine", tryNewLineRule, doNewLineRule},
			TokenRule{"typography", tryTypographyRule, doTypographyRule},
			TokenRule{"prelineSpace", tryPrelineSpaceRule, doPrelineSpaceRule},
			TokenRule{"plainText", tryPlainTextRule, doPlainTextRule}
		};
	}
	
	TokenedPage tokenizePage(std::string page){
		TokenRuleContext context = applyTokenizingRules(std::move(page), standardRules);
		
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
	
	namespace{
		bool check(const std::string& buffer, std::size_t pos, std::string text){
			if(pos + text.size() > buffer.size()){
				return false;
			}
			std::string temp = buffer.substr(pos, text.size());
			return text == temp;
		}
	}
	
	std::vector<std::string> getPageLinks(std::string page){
		std::vector<TokenRule> rules = {
			TokenRule{"comment", tryCommentRule, doCommentRule},
			TokenRule{"tripleLink", tryTripleLinkRule, doTripleLinkRule},
			TokenRule{"singleLink", trySingleLinkRule, doSingleLinkRule},
			TokenRule{"null", tryNullRule, doNullRule}
		};
		
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










