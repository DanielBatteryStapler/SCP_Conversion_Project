#ifndef TABLERULESET_HPP
#define TABLERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenTableMarker(const TokenVariant& tok);
    nlohmann::json printNodeTable(const NodeVariant& nod);
    nlohmann::json printNodeTableRow(const NodeVariant& nod);
    nlohmann::json printNodeTableElement(const NodeVariant& nod);
    
    nlohmann::json printNodeAdvTable(const NodeVariant& nod);
    nlohmann::json printNodeAdvTableRow(const NodeVariant& nod);
    nlohmann::json printNodeAdvTableElement(const NodeVariant& nod);
    
	bool tryTableMarkerRule(const TokenRuleContext& context);
	TokenRuleResult doTableMarkerRule(const TokenRuleContext& context);
	
    void handleTableMarker(TreeContext& context, const Token& token);
	
	void handleAdvTable(TreeContext& context, const Token& token);
	void handleAdvTableRow(TreeContext& context, const Token& token);
	void handleAdvTableElement(TreeContext& context, const Token& token);
	
	void toHtmlNodeTable(const HtmlContext& con, const Node& nod);
	void toHtmlNodeTableRow(const HtmlContext& con, const Node& nod);
	void toHtmlNodeTableElement(const HtmlContext& con, const Node& nod);
	
	void toHtmlNodeAdvTable(const HtmlContext& con, const Node& nod);
	void toHtmlNodeAdvTableRow(const HtmlContext& con, const Node& nod);
	void toHtmlNodeAdvTableElement(const HtmlContext& con, const Node& nod);
	
	const inline auto tableRuleSet = RuleSet{"Table", {
		TokenPrintRule{Token::Type::TableMarker, printTokenTableMarker},
		NodePrintRule{Node::Type::Table, printNodeTable},
		NodePrintRule{Node::Type::TableRow, printNodeTableRow},
		NodePrintRule{Node::Type::TableElement, printNodeTableElement},
		
		NodePrintRule{Node::Type::AdvTable, printNodeAdvTable},
		NodePrintRule{Node::Type::AdvTableRow, printNodeAdvTableRow},
		NodePrintRule{Node::Type::AdvTableElement, printNodeAdvTableElement},
		
		TokenRule{tryTableMarkerRule, doTableMarkerRule},
		
		SectionRule{SectionType::AdvTable, {"table"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
        SectionRule{SectionType::AdvTableRow, {"row"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
        SectionRule{SectionType::AdvTableElement, {"cell", "hcell"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
		
		TreeRule{{Token::Type::TableMarker}, handleTableMarker},
		
		TreeRule{{Token::Type::Section, SectionType::AdvTable}, handleAdvTable},
		TreeRule{{Token::Type::Section, SectionType::AdvTableRow}, handleAdvTableRow},
		TreeRule{{Token::Type::Section, SectionType::AdvTableElement}, handleAdvTableElement},
		
		HtmlRule{Node::Type::Table, toHtmlNodeTable},
		HtmlRule{Node::Type::TableRow, toHtmlNodeTableRow},
		HtmlRule{Node::Type::TableElement, toHtmlNodeTableElement},
		
		HtmlRule{Node::Type::AdvTable, toHtmlNodeAdvTable},
		HtmlRule{Node::Type::AdvTableRow, toHtmlNodeAdvTableRow},
		HtmlRule{Node::Type::AdvTableElement, toHtmlNodeAdvTableElement}
	}};	
}

#endif // TABLERULESET_HPP
