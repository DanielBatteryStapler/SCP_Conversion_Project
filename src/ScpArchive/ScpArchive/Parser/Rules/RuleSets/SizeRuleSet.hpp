#ifndef SIZERULESET_HPP
#define SIZERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringNodeSize(const NodeVariant& nod);
	
    void handleSize(TreeContext& context, const Token& token);
	
	void toHtmlNodeSize(const HtmlContext& con, const Node& nod);
	
	const inline auto sizeRuleSet = RuleSet{"Size", {
        NodePrintRule{Node::Type::Size, toStringNodeSize},
        
        TreeRule{{Token::Type::Section, SectionType::Size}, handleSize},
		
        SectionRule{SectionType::Size, {"size"}, SubnameType::Parameter, ModuleType::Unknown, {},
            ContentType::Surround, ParameterType::None, true},
        
        HtmlRule{Node::Type::Size, toHtmlNodeSize}
	}};
	
}

#endif // SIZERULESET_HPP
