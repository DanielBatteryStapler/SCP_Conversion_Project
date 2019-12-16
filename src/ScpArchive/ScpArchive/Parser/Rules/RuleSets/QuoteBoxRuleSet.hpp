#ifndef QUOTEBOXRULESET_HPP
#define QUOTEBOXRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenQuoteBoxPrefix(const TokenVariant& tok);
    std::string toStringNodeQuoteBox(const NodeVariant& nod);
	
	bool tryQuoteBoxPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doQuoteBoxPrefixRule(const TokenRuleContext& context);
	
	void toHtmlNodeQuoteBox(const HtmlContext& con, const Node& nod);
	
	const inline auto quoteBoxRuleSet = RuleSet{"QuoteBox", {
		TokenPrintRule{Token::Type::QuoteBoxPrefix, toStringTokenQuoteBoxPrefix},
		NodePrintRule{Node::Type::QuoteBox, toStringNodeQuoteBox},
		
		TokenRule{tryQuoteBoxPrefixRule, doQuoteBoxPrefixRule},
		
		HtmlRule{Node::Type::QuoteBox, toHtmlNodeQuoteBox}
	}};
	
}

#endif // QUOTEBOXRULESET_HPP
