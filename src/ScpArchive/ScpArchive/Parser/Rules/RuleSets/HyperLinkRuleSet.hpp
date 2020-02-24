#ifndef HYPERLINKRULESET_HPP
#define HYPERLINKRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenHyperLink(const TokenVariant& tok);
    nlohmann::json printNodeHyperLink(const NodeVariant& nod);
    
	bool tryTripleLinkRule(const TokenRuleContext& context);
	TokenRuleResult doTripleLinkRule(const TokenRuleContext& context);
	
	const inline auto tripleHyperLinkRuleSet = RuleSet{"TripleHyperLink", {
        TokenRule{tryTripleLinkRule, doTripleLinkRule}
	}};
	
	bool trySingleLinkRule(const TokenRuleContext& context);
	TokenRuleResult doSingleLinkRule(const TokenRuleContext& context);
	
	bool tryBareLinkRule(const TokenRuleContext& context);
	TokenRuleResult doBareLinkRule(const TokenRuleContext& context);
	
    void handleHyperLink(TreeContext& context, const Token& token);
    
    void toHtmlNodeHyperLink(const HtmlContext& con, const Node& nod);
	
	const inline auto hyperLinkRuleSet = RuleSet{"HyperLink", {
        TokenPrintRule{Token::Type::HyperLink, printTokenHyperLink},
        NodePrintRule{Node::Type::HyperLink, printNodeHyperLink},
        
        TokenRule{trySingleLinkRule, doSingleLinkRule},
        TokenRule{tryBareLinkRule, doBareLinkRule},
        
        TreeRule{{Token::Type::HyperLink}, handleHyperLink},
        
        HtmlRule{Node::Type::HyperLink, toHtmlNodeHyperLink}
	}};
	
}

#endif // HYPERLINKRULESET_HPP
