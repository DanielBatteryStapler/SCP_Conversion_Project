#ifndef DIVIDERRULESET_HPP
#define DIVIDERRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenDivider(const TokenVariant& tok);
    std::string toStringNodeDivider(const NodeVariant& nod);
    
	bool tryDividerRule(const TokenRuleContext& context);
	TokenRuleResult doDividerRule(const TokenRuleContext& context);
	
    void handleDivider(TreeContext& context, const Token& token);
	
	void toHtmlNodeDivider(const HtmlContext& con, const Node& nod);
	
	const inline auto dividerRuleSet = RuleSet{"Divider", {
		TokenPrintRule{Token::Type::Divider, toStringTokenDivider},
		NodePrintRule{Node::Type::Divider, toStringNodeDivider},
		
		TokenRule{tryDividerRule, doDividerRule},
		
        TreeRule{{Token::Type::Divider}, handleDivider},
        
        HtmlRule{Node::Type::Divider, toHtmlNodeDivider}
	}};
	
}

#endif // DIVIDERRULESET_HPP
