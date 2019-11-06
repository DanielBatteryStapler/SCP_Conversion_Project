#ifndef TOKENRULES_HPP
#define TOKENRULES_HPP

#include "Parser.hpp"

namespace Parser{
	bool tryCommentRule(const TokenRuleContext& context);
	TokenRuleResult doCommentRule(const TokenRuleContext& context);
		
	bool tryEntityEscapeRule(const TokenRuleContext& context);
	TokenRuleResult doEntityEscapeRule(const TokenRuleContext& context);
	
	bool tryLiteralTextRule(const TokenRuleContext& context);
	TokenRuleResult doLiteralTextRule(const TokenRuleContext& context);
	
	bool tryLineBreakRule(const TokenRuleContext& context);
	TokenRuleResult doLineBreakRule(const TokenRuleContext& context);
	
	bool tryEscapedNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doEscapedNewLineRule(const TokenRuleContext& context);
	
	bool tryNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doNewLineRule(const TokenRuleContext& context);
	
	bool tryTypographyRule(const TokenRuleContext& context);
	TokenRuleResult doTypographyRule(const TokenRuleContext& context);
	
	bool tryPlainTextRule(const TokenRuleContext& context);
	TokenRuleResult doPlainTextRule(const TokenRuleContext& context);
}

#endif // TOKENRULES_HPP
