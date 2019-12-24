#ifndef TABLERULESET_HPP
#define TABLERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenTableMarker(const TokenVariant& tok);
    
    std::string toStringNodeTable(const NodeVariant& nod);
    std::string toStringNodeTableRow(const NodeVariant& nod);
    std::string toStringNodeTableElement(const NodeVariant& nod);
    
	bool tryTableMarkerRule(const TokenRuleContext& context);
	TokenRuleResult doTableMarkerRule(const TokenRuleContext& context);
	
    void handleTableMarker(TreeContext& context, const Token& token);
	
	void toHtmlNodeTable(const HtmlContext& con, const Node& nod);
	void toHtmlNodeTableRow(const HtmlContext& con, const Node& nod);
	void toHtmlNodeTableElement(const HtmlContext& con, const Node& nod);
	
	const inline auto tableRuleSet = RuleSet{"Table", {
		TokenPrintRule{Token::Type::TableMarker, toStringTokenTableMarker},
		NodePrintRule{Node::Type::Table, toStringNodeTable},
		NodePrintRule{Node::Type::TableRow, toStringNodeTableRow},
		NodePrintRule{Node::Type::TableElement, toStringNodeTableElement},
		
		TokenRule{tryTableMarkerRule, doTableMarkerRule},
		
		TreeRule{{Token::Type::TableMarker}, handleTableMarker},
		
		HtmlRule{Node::Type::Table, toHtmlNodeTable},
		HtmlRule{Node::Type::TableRow, toHtmlNodeTableRow},
		HtmlRule{Node::Type::TableElement, toHtmlNodeTableElement}
	}};	
}

#endif // TABLERULESET_HPP
