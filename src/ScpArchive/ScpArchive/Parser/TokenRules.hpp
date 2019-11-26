#ifndef TOKENRULES_HPP
#define TOKENRULES_HPP

#include "Parser.hpp"

namespace Parser{
	bool tryCommentRule(const TokenRuleContext& context);
	TokenRuleResult doCommentRule(const TokenRuleContext& context);
	
	bool tryHeadingRule(const TokenRuleContext& context);
	TokenRuleResult doHeadingRule(const TokenRuleContext& context);
	
	bool tryDividerRule(const TokenRuleContext& context);
	TokenRuleResult doDividerRule(const TokenRuleContext& context);
	
	bool tryQuoteBoxPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doQuoteBoxPrefixRule(const TokenRuleContext& context);
	
	bool tryListPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doListPrefixRule(const TokenRuleContext& context);
	
	bool tryStrikeRule(const TokenRuleContext& context);
	TokenRuleResult doStrikeRule(const TokenRuleContext& context);
	bool tryItalicsRule(const TokenRuleContext& context);
	TokenRuleResult doItalicsRule(const TokenRuleContext& context);
	bool tryBoldRule(const TokenRuleContext& context);
	TokenRuleResult doBoldRule(const TokenRuleContext& context);
	bool tryUnderlineRule(const TokenRuleContext& context);
	TokenRuleResult doUnderlineRule(const TokenRuleContext& context);
	bool trySuperRule(const TokenRuleContext& context);
	TokenRuleResult doSuperRule(const TokenRuleContext& context);
	bool trySubRule(const TokenRuleContext& context);
	TokenRuleResult doSubRule(const TokenRuleContext& context);
	bool tryMonospaceRule(const TokenRuleContext& context);
	TokenRuleResult doMonospaceRule(const TokenRuleContext& context);
	bool tryColorRule(const TokenRuleContext& context);
	TokenRuleResult doColorRule(const TokenRuleContext& context);
	
	bool tryTripleLinkRule(const TokenRuleContext& context);
	TokenRuleResult doTripleLinkRule(const TokenRuleContext& context);
	
	bool trySingleLinkRule(const TokenRuleContext& context);
	TokenRuleResult doSingleLinkRule(const TokenRuleContext& context);
	
	bool tryBareLinkRule(const TokenRuleContext& context);
	TokenRuleResult doBareLinkRule(const TokenRuleContext& context);
	
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
	
	bool tryPrelineSpaceRule(const TokenRuleContext& context);
	TokenRuleResult doPrelineSpaceRule(const TokenRuleContext& context);
	
	bool tryPlainTextRule(const TokenRuleContext& context);
	TokenRuleResult doPlainTextRule(const TokenRuleContext& context);
	
	bool tryNullRule(const TokenRuleContext& context);
	TokenRuleResult doNullRule(const TokenRuleContext& context);
}

#endif // TOKENRULES_HPP
