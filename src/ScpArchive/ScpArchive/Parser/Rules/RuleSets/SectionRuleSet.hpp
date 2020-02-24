#ifndef SECTIONRULESET_HPP
#define SECTIONRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenSection(const TokenVariant& tok);
    nlohmann::json printTokenSectionStart(const TokenVariant& tok);
    nlohmann::json printTokenSectionEnd(const TokenVariant& tok);
    nlohmann::json printTokenSectionComplete(const TokenVariant& tok);
	
	bool trySectionRule(const TokenRuleContext& context);
	TokenRuleResult doSectionRule(const TokenRuleContext& context);
	
	const inline auto sectionRuleSet = RuleSet{"Section", {
	    TokenPrintRule{Token::Type::Section, printTokenSection},
	    TokenPrintRule{Token::Type::SectionStart, printTokenSectionStart},
	    TokenPrintRule{Token::Type::SectionEnd, printTokenSectionEnd},
	    TokenPrintRule{Token::Type::SectionComplete, printTokenSectionComplete},
	    
		TokenRule{trySectionRule, doSectionRule}
	}};
	
}

#endif // SECTIONRULESET_HPP
