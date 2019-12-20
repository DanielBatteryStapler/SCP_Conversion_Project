#ifndef IMAGERULESET_HPP
#define IMAGERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	std::string toStringNodeImage(const NodeVariant& nod);
	
    void handleImage(TreeContext& context, const Token& token);
	
	void toHtmlNodeImage(const HtmlContext& con, const Node& nod);
	
	const inline auto imageRuleSet = RuleSet{"Image", {
		NodePrintRule{Node::Type::Image, toStringNodeImage},
		
		SectionRule{SectionType::Image, {"image", "=image", "<image", ">image", "f<image", "f>image"}, SubnameType::Parameter, ModuleType::Unknown, {},
            ContentType::None, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Image}, handleImage},
        
        HtmlRule{Node::Type::Image, toHtmlNodeImage}
	}};
	
}

#endif // IMAGERULESET_HPP
