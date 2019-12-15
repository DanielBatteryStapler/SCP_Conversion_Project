#ifndef ALIGNRULESET_HPP
#define ALIGNRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	std::string toStringNodeAlign(const NodeVariant& nod);
	
    void handleAlign(TreeContext& context, const Token& token);
	
	const inline auto alignRuleSet = RuleSet{"Align", {
		NodePrintRule{Node::Type::Align, toStringNodeAlign},
		
		SectionRule{SectionType::Align, {"<", ">", "=", "=="}, SubnameType::None, ModuleType::Unknown, {},
            ContentType::Surround, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::Align}, handleAlign},
	}};
	
}

#endif // ALIGNRULESET_HPP
