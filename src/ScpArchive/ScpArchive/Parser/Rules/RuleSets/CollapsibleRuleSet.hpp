#ifndef COLLAPSIBLERULESET_HPP
#define COLLAPSIBLERULESET_HPP


#include "../RuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeCollapsible(const NodeVariant& nod);
    
    void handleCollapsible(TreeContext& context, const Token& token);
	
	void toHtmlNodeCollapsible(const HtmlContext& con, const Node& nod);
	
	const inline auto collapsibleRuleSet = RuleSet{"Collapsible", {
	    NodePrintRule{Node::Type::Collapsible, printNodeCollapsible},
	    
		SectionRule{SectionType::Collapsible, {"collapsible"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, false},
        
        TreeRule{{Token::Type::Section, SectionType::Collapsible}, handleCollapsible},
        
        HtmlRule{Node::Type::Collapsible, toHtmlNodeCollapsible}
	}};
	
}

#endif // COLLAPSIBLERULESET_HPP
