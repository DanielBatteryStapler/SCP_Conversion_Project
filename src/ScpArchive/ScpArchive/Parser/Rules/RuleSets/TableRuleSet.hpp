#ifndef TABLERULESET_HPP
#define TABLERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenTableMarker(const TokenVariant& tok);
    
    std::string toStringNodeTable(const NodeVariant& nod);
    std::string toStringNodeTableRow(const NodeVariant& nod);
    std::string toStringNodeTableElement(const NodeVariant& nod);
    
    std::string toStringNodeAdvTable(const NodeVariant& nod);
    std::string toStringNodeAdvTableRow(const NodeVariant& nod);
    std::string toStringNodeAdvTableElement(const NodeVariant& nod);
    
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
		TokenPrintRule{Token::Type::TableMarker, toStringTokenTableMarker},
		NodePrintRule{Node::Type::Table, toStringNodeTable},
		NodePrintRule{Node::Type::TableRow, toStringNodeTableRow},
		NodePrintRule{Node::Type::TableElement, toStringNodeTableElement},
		
		NodePrintRule{Node::Type::AdvTable, toStringNodeAdvTable},
		NodePrintRule{Node::Type::AdvTableRow, toStringNodeAdvTableRow},
		NodePrintRule{Node::Type::AdvTableElement, toStringNodeAdvTableElement},
		
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
		HtmlRule{Node::Type::TableElement, toHtmlNodeTableElement}
	}};	
}

#endif // TABLERULESET_HPP
