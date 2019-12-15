#ifndef SECTIONRULESET_HPP
#define SECTIONRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenSection(const TokenVariant& tok);
    std::string toStringTokenSectionStart(const TokenVariant& tok);
    std::string toStringTokenSectionEnd(const TokenVariant& tok);
    std::string toStringTokenSectionComplete(const TokenVariant& tok);
	
	bool trySectionRule(const TokenRuleContext& context);
	TokenRuleResult doSectionRule(const TokenRuleContext& context);
	
	const inline auto sectionRuleSet = RuleSet{"Section", {
	    TokenPrintRule{Token::Type::Section, toStringTokenSection},
	    TokenPrintRule{Token::Type::SectionStart, toStringTokenSectionStart},
	    TokenPrintRule{Token::Type::SectionEnd, toStringTokenSectionEnd},
	    TokenPrintRule{Token::Type::SectionComplete, toStringTokenSectionComplete},
	    
		TokenRule{trySectionRule, doSectionRule}
	}};
	
}

#endif // SECTIONRULESET_HPP
