#ifndef INLINEFORMATRULESET_HPP
#define INLINEFORMATRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenInlineFormat(const TokenVariant& tok);
    nlohmann::json printNodeStyleFormat(const NodeVariant& nod);
    
	bool tryStrikeRule(const TokenRuleContext& context);
	TokenRuleResult doStrikeRule(const TokenRuleContext& context);
	bool tryItalicsRule(const TokenRuleContext& context);
	TokenRuleResult doItalicsRule(const TokenRuleContext& context);
	bool tryBoldRule(const TokenRuleContext& context);
	TokenRuleResult doBoldRule(const TokenRuleContext& context);
	bool tryUnderlineRule(const TokenRuleContext& context);
	TokenRuleResult doUnderlineRule(const TokenRuleContext& context);
	bool trySuperRule(const TokenRuleContext& context);
	TokenRuleResult doSuperRule(const TokenRuleContext& context);
	bool trySubRule(const TokenRuleContext& context);
	TokenRuleResult doSubRule(const TokenRuleContext& context);
	bool tryMonospaceRule(const TokenRuleContext& context);
	TokenRuleResult doMonospaceRule(const TokenRuleContext& context);
	bool tryColorRule(const TokenRuleContext& context);
	TokenRuleResult doColorRule(const TokenRuleContext& context);
	
    void handleInlineFormat(TreeContext& context, const Token& token);
	
	void toHtmlNodeStyleFormat(const HtmlContext& con, const Node& nod);
	
	const inline auto inlineFormatRuleSet = RuleSet{"InlineFormat", {
	    TokenPrintRule{Token::Type::InlineFormat, printTokenInlineFormat},
	    NodePrintRule{Node::Type::StyleFormat, printNodeStyleFormat},
	    
		TokenRule{tryStrikeRule, doStrikeRule},
		TokenRule{tryItalicsRule, doItalicsRule},
		TokenRule{tryBoldRule, doBoldRule},
		TokenRule{tryUnderlineRule, doUnderlineRule},
		TokenRule{trySuperRule, doSuperRule},
		TokenRule{trySubRule, doSubRule},
		TokenRule{tryMonospaceRule, doMonospaceRule},
		TokenRule{tryColorRule, doColorRule},
		
        TreeRule{{Token::Type::InlineFormat}, handleInlineFormat},
        
        HtmlRule{Node::Type::StyleFormat, toHtmlNodeStyleFormat}
	}};
	
}

#endif // INLINEFORMATRULESET_HPP
