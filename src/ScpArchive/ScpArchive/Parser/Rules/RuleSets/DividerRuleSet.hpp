#ifndef DIVIDERRULESET_HPP
#define DIVIDERRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenDivider(const TokenVariant& tok);
    nlohmann::json printNodeDivider(const NodeVariant& nod);
    
	bool tryDividerRule(const TokenRuleContext& context);
	TokenRuleResult doDividerRule(const TokenRuleContext& context);
	
    void handleDivider(TreeContext& context, const Token& token);
	
	void toHtmlNodeDivider(const HtmlContext& con, const Node& nod);
	
	const inline auto dividerRuleSet = RuleSet{"Divider", {
		TokenPrintRule{Token::Type::Divider, printTokenDivider},
		NodePrintRule{Node::Type::Divider, printNodeDivider},
		
		TokenRule{tryDividerRule, doDividerRule},
		
        TreeRule{{Token::Type::Divider}, handleDivider},
        
        HtmlRule{Node::Type::Divider, toHtmlNodeDivider}
	}};
	
}

#endif // DIVIDERRULESET_HPP
