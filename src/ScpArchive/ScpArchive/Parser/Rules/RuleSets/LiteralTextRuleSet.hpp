#ifndef LITERALTEXTRULESET_HPP
#define LITERALTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	bool tryLiteralTextRule(const TokenRuleContext& context);
	TokenRuleResult doLiteralTextRule(const TokenRuleContext& context);
	
    void handleLiteralText(TreeContext& context, const Token& token);
	
	const inline auto literalTextRuleSet = RuleSet{"LiteralText", {
		TokenRule{tryLiteralTextRule, doLiteralTextRule},
		
        TreeRule{{Token::Type::LiteralText}, handleLiteralText}
	}};
	
}

#endif // LITERALTEXTRULESET_HPP
