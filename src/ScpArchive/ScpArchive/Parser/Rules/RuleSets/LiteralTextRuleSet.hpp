#ifndef LITERALTEXTRULESET_HPP
#define LITERALTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenLiteralText(const TokenVariant& tok);
    std::string toStringNodeLiteralText(const NodeVariant& nod);
	
	bool tryLiteralTextRule(const TokenRuleContext& context);
	TokenRuleResult doLiteralTextRule(const TokenRuleContext& context);
	
    void handleLiteralText(TreeContext& context, const Token& token);
	
	const inline auto literalTextRuleSet = RuleSet{"LiteralText", {
	    TokenPrintRule{Token::Type::LiteralText, toStringTokenLiteralText},
	    NodePrintRule{Node::Type::LiteralText, toStringNodeLiteralText},
	    
		TokenRule{tryLiteralTextRule, doLiteralTextRule},
		
        TreeRule{{Token::Type::LiteralText}, handleLiteralText}
	}};
	
}

#endif // LITERALTEXTRULESET_HPP
