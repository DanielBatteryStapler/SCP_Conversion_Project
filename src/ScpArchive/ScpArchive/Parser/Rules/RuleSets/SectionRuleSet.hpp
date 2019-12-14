#ifndef SECTIONRULESET_HPP
#define SECTIONRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	bool trySectionRule(const TokenRuleContext& context);
	TokenRuleResult doSectionRule(const TokenRuleContext& context);
	
	const inline auto sectionRuleSet = RuleSet{"Section", {
		TokenRule{trySectionRule, doSectionRule}
	}};
	
}

#endif // SECTIONRULESET_HPP
