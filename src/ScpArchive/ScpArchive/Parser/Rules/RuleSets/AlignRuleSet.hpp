#ifndef ALIGNRULESET_HPP
#define ALIGNRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	nlohmann::json printNodeAlign(const NodeVariant& nod);
	
    void handleAlign(TreeContext& context, const Token& token);
	
	void toHtmlNodeAlign(const HtmlContext& con, const Node& nod);
	
	const inline auto alignRuleSet = RuleSet{"Align", {
		NodePrintRule{Node::Type::Align, printNodeAlign},
		
		SectionRule{SectionType::Align, {"<", ">", "=", "=="}, SubnameType::None, ModuleType::Unknown, {},
            ContentType::Surround, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::Align}, handleAlign},
        
        HtmlRule{Node::Type::Align, toHtmlNodeAlign}
	}};
	
}

#endif // ALIGNRULESET_HPP
