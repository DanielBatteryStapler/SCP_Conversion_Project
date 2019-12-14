#ifndef NULLRULESET_HPP
#define NULLRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    bool tryNullRule(const TokenRuleContext& context);
	TokenRuleResult doNullRule(const TokenRuleContext& context);
	
	const inline auto nullRuleSet = RuleSet{"Null", {
		TokenRule{tryNullRule, doNullRule}
	}};
}

#endif // NULLRULESET_HPP
