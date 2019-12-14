#ifndef LISTRULESET_HPP
#define LISTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	bool tryListPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doListPrefixRule(const TokenRuleContext& context);
	
	const inline auto listRuleSet = RuleSet{"List", {
		TokenRule{tryListPrefixRule, doListPrefixRule}
	}};
	
}

#endif // LISTRULESET_HPP
