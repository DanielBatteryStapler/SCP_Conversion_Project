#ifndef BASICTEXTRULESET_HPP
#define BASICTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    bool tryCarriageReturn(const TokenRuleContext& context);
	TokenRuleResult doCarriageReturn(const TokenRuleContext& context);
	
	bool tryLineBreakRule(const TokenRuleContext& context);
	TokenRuleResult doLineBreakRule(const TokenRuleContext& context);
	
	bool tryEscapedNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doEscapedNewLineRule(const TokenRuleContext& context);
	
	bool tryNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doNewLineRule(const TokenRuleContext& context);
	
	bool tryTypographyRule(const TokenRuleContext& context);
	TokenRuleResult doTypographyRule(const TokenRuleContext& context);
	
	bool tryPrelineSpaceRule(const TokenRuleContext& context);
	TokenRuleResult doPrelineSpaceRule(const TokenRuleContext& context);
	
	bool tryPlainTextRule(const TokenRuleContext& context);
	TokenRuleResult doPlainTextRule(const TokenRuleContext& context);
    
    void handleLineBreak(TreeContext& context, const Token& token);
    void handlePlainText(TreeContext& context, const Token& token);
    
    const inline auto basicTextRuleSet = RuleSet{"BasicText", {
		TokenRule{tryCarriageReturn, doCarriageReturn},
		TokenRule{tryLineBreakRule, doLineBreakRule},
		TokenRule{tryEscapedNewLineRule, doEscapedNewLineRule},
		TokenRule{tryNewLineRule, doNewLineRule},
		TokenRule{tryTypographyRule, doTypographyRule},
		TokenRule{tryPrelineSpaceRule, doPrelineSpaceRule},
		TokenRule{tryPlainTextRule, doPlainTextRule},
		
		TreeRule{{Token::Type::PlainText}, handlePlainText},
		TreeRule{{Token::Type::LineBreak}, handleLineBreak}
	}};
}

#endif // BASICTEXTRULESET_HPP
