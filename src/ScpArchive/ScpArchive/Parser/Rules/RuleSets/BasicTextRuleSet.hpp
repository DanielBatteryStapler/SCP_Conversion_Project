#ifndef BASICTEXTRULESET_HPP
#define BASICTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenPlainText(const TokenVariant& tok);
    nlohmann::json printTokenNewLine(const TokenVariant& tok);
    nlohmann::json printTokenLineBreak(const TokenVariant& tok);
    
    nlohmann::json printNodePlainText(const NodeVariant& nod);
    nlohmann::json printNodeParagraph(const NodeVariant& nod);
    nlohmann::json printNodeLineBreak(const NodeVariant& nod);
    nlohmann::json printNodeRootPage(const NodeVariant& nod);
    
    bool tryCarriageReturn(const TokenRuleContext& context);
	TokenRuleResult doCarriageReturn(const TokenRuleContext& context);
	
	bool tryLineBreakRule(const TokenRuleContext& context);
	TokenRuleResult doLineBreakRule(const TokenRuleContext& context);
	
	bool tryEscapedNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doEscapedNewLineRule(const TokenRuleContext& context);
	
	bool tryNewLineRule(const TokenRuleContext& context);
	TokenRuleResult doNewLineRule(const TokenRuleContext& context);
	
	bool tryTypographyRule(const TokenRuleContext& context);
	TokenRuleResult doTypographyRule(const TokenRuleContext& context);
	
	bool tryPrelineSpaceRule(const TokenRuleContext& context);
	TokenRuleResult doPrelineSpaceRule(const TokenRuleContext& context);
	
	bool tryPlainTextRule(const TokenRuleContext& context);
	TokenRuleResult doPlainTextRule(const TokenRuleContext& context);
    
    void handleLineBreak(TreeContext& context, const Token& token);
    void handlePlainText(TreeContext& context, const Token& token);
    
	void toHtmlNodeRootPage(const HtmlContext& con, const Node& nod);
	void toHtmlNodeParagraph(const HtmlContext& con, const Node& nod);
	void toHtmlNodeLineBreak(const HtmlContext& con, const Node& nod);
	void toHtmlNodePlainText(const HtmlContext& con, const Node& nod);
    
    const inline auto basicTextRuleSet = RuleSet{"BasicText", {
		TokenPrintRule{Token::Type::PlainText, printTokenPlainText},
		TokenPrintRule{Token::Type::NewLine, printTokenNewLine},
		TokenPrintRule{Token::Type::LineBreak, printTokenLineBreak},
		
		NodePrintRule{Node::Type::PlainText, printNodePlainText},
		NodePrintRule{Node::Type::Paragraph, printNodeParagraph},
		NodePrintRule{Node::Type::LineBreak, printNodeLineBreak},
		NodePrintRule{Node::Type::RootPage, printNodeRootPage},
		
		TokenRule{tryCarriageReturn, doCarriageReturn},
		TokenRule{tryLineBreakRule, doLineBreakRule},
		TokenRule{tryEscapedNewLineRule, doEscapedNewLineRule},
		TokenRule{tryNewLineRule, doNewLineRule},
		TokenRule{tryTypographyRule, doTypographyRule},
		TokenRule{tryPrelineSpaceRule, doPrelineSpaceRule},
		TokenRule{tryPlainTextRule, doPlainTextRule},
		
		TreeRule{{Token::Type::PlainText}, handlePlainText},
		TreeRule{{Token::Type::LineBreak}, handleLineBreak},
		
		HtmlRule{Node::Type::RootPage, toHtmlNodeRootPage},
		HtmlRule{Node::Type::Paragraph, toHtmlNodeParagraph},
		HtmlRule{Node::Type::LineBreak, toHtmlNodeLineBreak},
		HtmlRule{Node::Type::PlainText, toHtmlNodePlainText}
	}};
}

#endif // BASICTEXTRULESET_HPP
