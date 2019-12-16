#ifndef BASICTEXTRULESET_HPP
#define BASICTEXTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenPlainText(const TokenVariant& tok);
    std::string toStringTokenNewLine(const TokenVariant& tok);
    std::string toStringTokenLineBreak(const TokenVariant& tok);
    
    std::string toStringNodePlainText(const NodeVariant& nod);
    std::string toStringNodeParagraph(const NodeVariant& nod);
    std::string toStringNodeLineBreak(const NodeVariant& nod);
    std::string toStringNodeRootPage(const NodeVariant& nod);
    
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
		TokenPrintRule{Token::Type::PlainText, toStringTokenPlainText},
		TokenPrintRule{Token::Type::NewLine, toStringTokenNewLine},
		TokenPrintRule{Token::Type::LineBreak, toStringTokenLineBreak},
		
		NodePrintRule{Node::Type::PlainText, toStringNodePlainText},
		NodePrintRule{Node::Type::Paragraph, toStringNodeParagraph},
		NodePrintRule{Node::Type::LineBreak, toStringNodeLineBreak},
		NodePrintRule{Node::Type::RootPage, toStringNodeRootPage},
		
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
