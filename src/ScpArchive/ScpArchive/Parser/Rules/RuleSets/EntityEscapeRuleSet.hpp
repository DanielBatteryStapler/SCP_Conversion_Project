#ifndef ENTITYESCAPERULESET_HPP
#define ENTITYESCAPERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	bool tryEntityEscapeRule(const TokenRuleContext& context);
	TokenRuleResult doEntityEscapeRule(const TokenRuleContext& context);
	
	const inline auto entityEscapeRuleSet = RuleSet{"EntityEscape", {
		TokenRule{tryEntityEscapeRule, doEntityEscapeRule}
	}};
	
}

#endif // ENTITYESCAPERULESET_HPP
