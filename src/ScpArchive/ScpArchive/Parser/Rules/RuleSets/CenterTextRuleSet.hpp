#ifndef CENTERTEXTRULESET_HPP
#define CENTERTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenCenterText(const TokenVariant& tok);
    nlohmann::json printNodeCenterText(const NodeVariant& nod);
    
	bool tryCenterTextRule(const TokenRuleContext& context);
	TokenRuleResult doCenterTextRule(const TokenRuleContext& context);
	
    void handleCenterText(TreeContext& context, const Token& token);
	
	void toHtmlNodeCenterText(const HtmlContext& con, const Node& nod);
	
	const inline auto centerTextRuleSet = RuleSet{"CenterText", {
		TokenPrintRule{Token::Type::CenterText, printTokenCenterText},
		NodePrintRule{Node::Type::CenterText, printNodeCenterText},
		
		TokenRule{tryCenterTextRule, doCenterTextRule},
		
        TreeRule{{Token::Type::CenterText}, handleCenterText},
        
        HtmlRule{Node::Type::CenterText, toHtmlNodeCenterText}
	}};
	
}

#endif // CENTERTEXTRULESET_HPP
