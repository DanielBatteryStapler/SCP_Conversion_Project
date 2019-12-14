#ifndef COMMENTRULESET_HPP
#define COMMENTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    bool tryCommentRule(const TokenRuleContext& context);
	TokenRuleResult doCommentRule(const TokenRuleContext& context);
	
	const inline auto commentRuleSet = RuleSet{"Comment", {
		TokenRule{tryCommentRule, doCommentRule}
	}};
}

#endif // COMMENTRULESET_HPP
