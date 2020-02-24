#include "Templater.hpp"

#include <sstream>

#include "Rules/RuleSetUtil.hpp"

namespace Parser{
    namespace{
        std::string replaceAll(std::string str, const std::string& from, const std::string& to){
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
            }
            return str;
        };
	}
    
    namespace{
        struct TemplateRule{
            std::string from;
            std::string to;
        };
    
        std::string applyTemplateRules(std::string page, const std::vector<TemplateRule>& rules){
            std::stringstream output;
            
            std::size_t pos = 0;
            while(pos < page.size()){
                bool ruleMatched = false;
                for(const TemplateRule& rule : rules){
                    if(check(page, pos, rule.from)){
                        ruleMatched = true;
                        output << rule.to;
                        pos += rule.from.size();
                    }
                }
                if(ruleMatched == false){
                    output << page[pos];
                    pos++;
                }
            }
            
            return output.str();
        }
    }
    
    std::string applyPageTemplate(std::string format, std::string page, PageInfo pageInfo){
        std::vector<TemplateRule> rules;
        rules.push_back({"%%content%%", page});
        TokenedPage tokens = tokenizePage(page, {pageInfo});
        
        {
            std::vector<std::string> contents;
            {
                std::size_t pos = 0;
                for(const Token& token : tokens.tokens){
                    if(token.getType() == Token::Type::Divider){
                        const Divider& divider = std::get<Divider>(token.token);
                        if(divider.type == Divider::Type::Separator){
                            contents.push_back(tokens.originalPage.substr(pos, token.sourceStart - pos));
                            pos = token.sourceEnd;
                        }
                    }
                }
                contents.push_back(tokens.originalPage.substr(pos, tokens.originalPage.size() - pos));
            }
            for(std::size_t i = 0; i < contents.size(); i++){
                rules.push_back({"%%content(" + std::to_string(i + 1) + ")%%", contents[i]});
            }
        }
        
        return applyTemplateRules(format, rules);
	}
	
    std::string applyIncludeParameters(std::string page, const std::map<std::string, std::string>& parameters){
		std::vector<TemplateRule> rules;
		for(auto i = parameters.begin(); i != parameters.end(); i++){
            rules.push_back({"{$" + i->first + "}", i->second});
		}
		return applyTemplateRules(page, rules);
    }
}
