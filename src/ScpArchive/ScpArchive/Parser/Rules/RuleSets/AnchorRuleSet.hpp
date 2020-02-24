#ifndef ANCHORRULESET_HPP
#define ANCHORRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	nlohmann::json printNodeAnchor(const NodeVariant& nod);
	
    void handleAnchor(TreeContext& context, const Token& token);
	
	void toHtmlNodeAnchor(const HtmlContext& con, const Node& nod);
	
	const inline auto anchorRuleSet = RuleSet{"Anchor", {
		NodePrintRule{Node::Type::Anchor, printNodeAnchor},
		
		SectionRule{SectionType::Anchor, {"#"}, SubnameType::Parameter, ModuleType::Unknown, {},
                ContentType::None, ParameterType::None, true},
		
        TreeRule{{Token::Type::Section, SectionType::Anchor}, handleAnchor},
        
        HtmlRule{Node::Type::Anchor, toHtmlNodeAnchor}
	}};
	
}

#endif // ANCHORRULESET_HPP
