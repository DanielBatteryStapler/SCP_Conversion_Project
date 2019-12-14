#ifndef HEADINGRULESET_HPP
#define HEADINGRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	bool tryHeadingRule(const TokenRuleContext& context);
	TokenRuleResult doHeadingRule(const TokenRuleContext& context);
	
    void handleHeading(TreeContext& context, const Token& token);
	
	const inline auto headingRuleSet = RuleSet{"Heading", {
		TokenRule{tryHeadingRule, doHeadingRule},
		
        TreeRule{{Token::Type::Heading}, handleHeading}
	}};
}

#endif // HEADINGRULESET_HPP
