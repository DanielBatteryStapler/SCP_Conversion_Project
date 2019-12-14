#ifndef HYPERLINKRULESET_HPP
#define HYPERLINKRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
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
	
	const inline auto hyperLinkRuleSet = RuleSet{"HyperLink", {
        TokenRule{trySingleLinkRule, doSingleLinkRule},
        TokenRule{tryBareLinkRule, doBareLinkRule},
        
        TreeRule{{Token::Type::HyperLink}, handleHyperLink}
	}};
	
}

#endif // HYPERLINKRULESET_HPP
