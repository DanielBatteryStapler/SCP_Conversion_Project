#ifndef HEADINGRULESET_HPP
#define HEADINGRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenHeading(const TokenVariant& tok);
    nlohmann::json printNodeHeading(const NodeVariant& nod);
	
	bool tryHeadingRule(const TokenRuleContext& context);
	TokenRuleResult doHeadingRule(const TokenRuleContext& context);
	
    void handleHeading(TreeContext& context, const Token& token);
    
	void toHtmlNodeHeading(const HtmlContext& con, const Node& nod);
	
	const inline auto headingRuleSet = RuleSet{"Heading", {
	    TokenPrintRule{Token::Type::Heading, printTokenHeading},
	    NodePrintRule{Node::Type::Heading, printNodeHeading},
	    
		TokenRule{tryHeadingRule, doHeadingRule},
		
        TreeRule{{Token::Type::Heading}, handleHeading},
        
        HtmlRule{Node::Type::Heading, toHtmlNodeHeading}
	}};
}

#endif // HEADINGRULESET_HPP
