#ifndef QUOTEBOXRULESET_HPP
#define QUOTEBOXRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenQuoteBoxPrefix(const TokenVariant& tok);
    nlohmann::json printNodeQuoteBox(const NodeVariant& nod);
	
	bool tryQuoteBoxPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doQuoteBoxPrefixRule(const TokenRuleContext& context);
	
	void toHtmlNodeQuoteBox(const HtmlContext& con, const Node& nod);
	
	const inline auto quoteBoxRuleSet = RuleSet{"QuoteBox", {
		TokenPrintRule{Token::Type::QuoteBoxPrefix, printTokenQuoteBoxPrefix},
		NodePrintRule{Node::Type::QuoteBox, printNodeQuoteBox},
		
		TokenRule{tryQuoteBoxPrefixRule, doQuoteBoxPrefixRule},
		
		HtmlRule{Node::Type::QuoteBox, toHtmlNodeQuoteBox}
	}};
	
}

#endif // QUOTEBOXRULESET_HPP
