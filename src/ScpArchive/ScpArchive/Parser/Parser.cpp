#include "Parser.hpp"

#include "Rules/RuleSet.hpp"
#include "Templater.hpp"

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
			else if(isspace(c) || c == '-' || c == '.' || c == '_'){
				if(output.size() == 0 || output.back() == ':' || output.back() == '-'){
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
		
	Token::Type Token::getType()const{
		return static_cast<Type>(token.index());
	}
	
	bool Token::operator==(const Token& tok)const{
		return printToken(*this) == printToken(tok);
	}
	
	std::string getTokenTypeName(Token::Type type){
		return TokenTypeNames.at(static_cast<std::size_t>(type));
	}
	
	nlohmann::json printTokenVariant(const Token& tok){
		const std::vector<TokenPrintRule> tokenPrintRules = getTokenPrintRules();
		Token::Type tokType = tok.getType();
		
		for(const TokenPrintRule& printRule : tokenPrintRules){
            if(printRule.type == tokType){
                return printRule.print(tok.token);
            }
		}
		throw std::runtime_error("Attempted to print a Token with no valid TokenPrintRule");
	}
	
	nlohmann::json printToken(const Token& tok){
		nlohmann::json out;
		out["type"] = getTokenTypeName(tok.getType());
		out["source"] = tok.source;
		out["sourceStart"] = tok.sourceStart;
		out["sourceEnd"] = tok.sourceEnd;
		out["data"] = printTokenVariant(tok);
		return out;
	}
	
	std::ostream& operator<<(std::ostream& out, const Token& tok){
		out << printToken(tok).dump(4);
		return out;
	}
	
	//#define BENCHMARKTOKENIZER
	#ifdef BENCHMARKTOKENIZER
}
#include <chrono>
#include <iomanip>
namespace Parser{
	#endif // BENCHMARKTOKENIZER
	
	namespace{
		TokenRuleContext applyTokenizingRules(std::string&& page, std::vector<TokenRule> rules){
			#ifdef BENCHMARKTOKENIZER
			using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
			const auto getTime = [](){return std::chrono::high_resolution_clock::now();};
			const auto getLength = [](TimePoint before, TimePoint after)->int64_t{return std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();};
			
			std::map<std::string, int64_t> tryTokenTiming;
			std::map<std::string, int64_t> doTokenTiming;
			#endif // BENCHMARKTOKENIZER
			
			TokenRuleContext context;
			context.page = page;
			context.pagePos = 0;
			context.wasNewLine = true;
			for(; context.pagePos < context.page.size();){
				bool allRulesFailed = true;
				for(const TokenRule& rule : rules){
					#ifdef BENCHMARKTOKENIZER
					TimePoint startTry = getTime();
					#endif // BENCHMARKTOKENIZER
					if(rule.tryRule(context)){
						#ifdef BENCHMARKTOKENIZER
						TimePoint endTry = getTime();
						tryTokenTiming[rule.parentRuleSet] += getLength(startTry, endTry);
						TimePoint startDo = getTime();
						#endif // BENCHMARKTOKENIZER
						allRulesFailed = false;
						
						TokenRuleResult result = rule.doRule(context);
						#ifdef BENCHMARKTOKENIZER
						TimePoint endDo = getTime();
						doTokenTiming[rule.parentRuleSet] += getLength(startDo, endDo);
						#endif // BENCHMARKTOKENIZER
						context.pagePos = result.newPos;
						context.wasNewLine = result.nowNewline;
						//make sure to merge plain text tokens together
						for(const Token& tok : result.newTokens){
                            if(context.tokens.size() > 0){
                                Token& back = context.tokens.back();
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
                            context.tokens.push_back(tok);
                        }
						//context.tokens.insert(context.tokens.end(), result.newTokens.begin(), result.newTokens.end());
						
						break;
					}
					#ifdef BENCHMARKTOKENIZER
					else{
						TimePoint endTry = getTime();
						tryTokenTiming[rule.parentRuleSet] += getLength(startTry, endTry);
					}	
					#endif // BENCHMARKTOKENIZER
				}
				if(allRulesFailed){
					throw std::runtime_error("All parsing rules failed, this should not be possible");
				}
			}
			
			#ifdef BENCHMARKTOKENIZER
			long double totalLength = 0;
			for(auto i = tryTokenTiming.begin(); i != tryTokenTiming.end(); i++){
				totalLength += i->second;
			}
			for(auto i = doTokenTiming.begin(); i != doTokenTiming.end(); i++){
				totalLength += i->second;
			}
			for(auto i = tryTokenTiming.begin(); i != tryTokenTiming.end(); i++){
				std::cout << "Checking for " << i->first << ": " << std::fixed << std::setprecision(4) << (i->second / totalLength * 100) << "%\n";
				auto doToken = doTokenTiming.find(i->first);
				if(doToken != doTokenTiming.end()){
					std::cout << "    Doing: " << std::fixed << std::setprecision(4) << (doToken->second / totalLength * 100) << "%\n";
				}
			}
			#endif // BENCHMARKTOKENIZER
			
			return context;
		}
	}
	
	namespace{
        std::string replaceAll(std::string str, const std::string& from, const std::string& to){
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        }
	}
	
	TokenedPage tokenizePage(std::string page, ParserParameters parameters){
		//just to save myself some headaches, lets replace all nonbreaking spaces with normal spaces
		//this might be refactored out in the future if the token rules are updated to deal with nonbreaking spaces more correctly
		page = replaceAll(page, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)}, " ");
		//replace tabs as well
		page = replaceAll(page, "\t", " ");
		
		page = applyIncludeParameters(page, parameters.includeParameters);
		
		TokenRuleContext context = applyTokenizingRules(std::move(page), getTokenRules());
		
		TokenedPage output;
		output.originalPage = std::move(context.page);
		output.tokens = std::move(context.tokens);
		
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
				link.url = redirectLink(link.url);
				if(!check(link.url, 0, "http://") && !check(link.url, 0, "https://")){
					while(link.url.size() > 0 && link.url[0] == '/'){
						link.url = link.url.substr(1, link.url.size() - 1);
					}
					if(link.url.find('/') != std::string::npos){
						link.url = link.url.substr(0, link.url.find('/'));
					}
					if(link.url != ""){
						links.push_back(link.url);
					}
				}
			}
		}
		
		return links;
	}
}










