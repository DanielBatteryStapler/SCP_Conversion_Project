#ifndef ARULESET_HPP
#define ARULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringNodeA(const NodeVariant& nod);
	
    void handleA(TreeContext& context, const Token& token);
	
	void toHtmlNodeA(const HtmlContext& con, const Node& nod);
	
	const inline auto aRuleSet = RuleSet{"A", {
        NodePrintRule{Node::Type::A, toStringNodeA},
        
        SectionRule{SectionType::A, {"a", "a_"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, true},
		
        TreeRule{{Token::Type::Section, SectionType::A}, handleA},
        
        HtmlRule{Node::Type::A, toHtmlNodeA}
	}};
	
}

#endif // ARULESET_HPP
