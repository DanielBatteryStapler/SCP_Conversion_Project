#ifndef DIVIDERRULESET_HPP
#define DIVIDERRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    
	bool tryDividerRule(const TokenRuleContext& context);
	TokenRuleResult doDividerRule(const TokenRuleContext& context);
	
    void handleDivider(TreeContext& context, const Token& token);
	
	const inline auto dividerRuleSet = RuleSet{"Divider", {
		TokenRule{tryDividerRule, doDividerRule},
		
        TreeRule{{Token::Type::Divider}, handleDivider}
	}};
	
}

#endif // DIVIDERRULESET_HPP
