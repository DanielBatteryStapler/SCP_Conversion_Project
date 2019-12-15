#ifndef HEADINGRULESET_HPP
#define HEADINGRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenHeading(const TokenVariant& tok);
    std::string toStringNodeHeading(const NodeVariant& nod);
	
	bool tryHeadingRule(const TokenRuleContext& context);
	TokenRuleResult doHeadingRule(const TokenRuleContext& context);
	
    void handleHeading(TreeContext& context, const Token& token);
	
	const inline auto headingRuleSet = RuleSet{"Heading", {
	    TokenPrintRule{Token::Type::Heading, toStringTokenHeading},
	    NodePrintRule{Node::Type::Heading, toStringNodeHeading},
	    
		TokenRule{tryHeadingRule, doHeadingRule},
		
        TreeRule{{Token::Type::Heading}, handleHeading}
	}};
}

#endif // HEADINGRULESET_HPP
