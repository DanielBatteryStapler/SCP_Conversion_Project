#ifndef QUOTEBOXRULESET_HPP
#define QUOTEBOXRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	bool tryQuoteBoxPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doQuoteBoxPrefixRule(const TokenRuleContext& context);
	
	const inline auto quoteBoxRuleSet = RuleSet{"QuoteBox", {
		TokenRule{tryQuoteBoxPrefixRule, doQuoteBoxPrefixRule}
	}};
	
}

#endif // QUOTEBOXRULESET_HPP
