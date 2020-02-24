#ifndef DIVRULESET_HPP
#define DIVRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeDiv(const NodeVariant& nod);
	
    void handleDiv(TreeContext& context, const Token& token);
	
	void toHtmlNodeDiv(const HtmlContext& con, const Node& nod);
	
	const inline auto divRuleSet = RuleSet{"Div", {
        NodePrintRule{Node::Type::Div, printNodeDiv},
        
        SectionRule{SectionType::Div, {"div", "div_"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Div}, handleDiv},
        
        HtmlRule{Node::Type::Div, toHtmlNodeDiv}
	}};
	
}

#endif // DIVRULESET_HPP
