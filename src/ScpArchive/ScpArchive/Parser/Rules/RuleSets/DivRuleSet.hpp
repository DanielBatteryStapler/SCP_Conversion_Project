#ifndef DIVRULESET_HPP
#define DIVRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringNodeDiv(const NodeVariant& nod);
	
    void handleDiv(TreeContext& context, const Token& token);
	
	const inline auto divRuleSet = RuleSet{"Div", {
        NodePrintRule{Node::Type::Div, toStringNodeDiv},
        
        SectionRule{SectionType::Div, {"div"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Div}, handleDiv}
	}};
	
}

#endif // DIVRULESET_HPP
