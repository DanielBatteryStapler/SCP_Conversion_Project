#ifndef HYPERLINKRULESET_HPP
#define HYPERLINKRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenHyperLink(const TokenVariant& tok);
    std::string toStringNodeHyperLink(const NodeVariant& nod);
    
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
        TokenPrintRule{Token::Type::HyperLink, toStringTokenHyperLink},
        NodePrintRule{Node::Type::HyperLink, toStringNodeHyperLink},
        
        TokenRule{trySingleLinkRule, doSingleLinkRule},
        TokenRule{tryBareLinkRule, doBareLinkRule},
        
        TreeRule{{Token::Type::HyperLink}, handleHyperLink},
        
        HtmlRule{Node::Type::HyperLink, toHtmlNodeHyperLink}
	}};
	
}

#endif // HYPERLINKRULESET_HPP
