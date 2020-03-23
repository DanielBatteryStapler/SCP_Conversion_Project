#include "Templater.hpp"

#include <sstream>

#include "Rules/RuleSetUtil.hpp"

namespace Parser{
    namespace{
        struct TemplateRule{
            std::string from;
            std::string to;
        };
    
        std::string applyTemplateRules(std::string pageContent, const std::vector<TemplateRule>& rules){
            std::stringstream output;
            
            std::size_t pos = 0;
            while(pos < pageContent.size()){
                bool ruleMatched = false;
                for(const TemplateRule& rule : rules){
                    if(check(pageContent, pos, rule.from)){///TODO: make this case in-sensitive
                        ruleMatched = true;
                        output << rule.to;
                        pos += rule.from.size();
                    }
                }
                if(ruleMatched == false){
                    output << pageContent[pos];
                    pos++;
                }
            }
            
            return output.str();
        }
    }
    
    std::string applyPageTemplate(std::string format, std::string pageContent, PageInfo pageInfo){
        std::vector<TemplateRule> rules;
        rules.push_back({"%%content%%", pageContent});
        rules.push_back({"%%name%%", pageInfo.name});//technically incorrect by the standard
        rules.push_back({"%%fullname%%", pageInfo.name});
        rules.push_back({"%%title%%", pageInfo.name});
        rules.push_back({"%%title_linked%%", "[/" + pageInfo.name + " " + pageInfo.title + "]"});
        rules.push_back({"%%rating%%", std::to_string(pageInfo.rating)});
        
        //The follow is commented out because it is extremely slow and I have never seen it used on the Wiki
        //I'm sure it has been, but I've never seen it. AFAIK it works perfectly fine, it's just really slow
        /*
        {
            std::vector<std::string> contents;
			TokenedPage tokens = tokenizePage(pageContent, {pageInfo});
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
            for(std::size_t i = 0; i < contents.size(); i++){
                rules.push_back({"%%content(" + std::to_string(i + 1) + ")%%", contents[i]});
            }
        }
        */
        
        return applyTemplateRules(format, rules);
	}
	
    std::string applyIncludeParameters(std::string pageContent, const std::map<std::string, std::string>& parameters){
		std::vector<TemplateRule> rules;
		for(auto i = parameters.begin(); i != parameters.end(); i++){
            rules.push_back({"{$" + i->first + "}", i->second});
		}
		return applyTemplateRules(pageContent, rules);
    }
}
