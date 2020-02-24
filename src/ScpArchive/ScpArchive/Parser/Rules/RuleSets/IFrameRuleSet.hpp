#ifndef IFRAMERULESET_HPP
#define IFRAMERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeIFrame(const NodeVariant& nod);
	
    void handleIFrame(TreeContext& context, const Token& token);
	
	void toHtmlNodeIFrame(const HtmlContext& con, const Node& nod);
	
	const inline auto iframeRuleSet = RuleSet{"IFrame", {
	    NodePrintRule{Node::Type::IFrame, printNodeIFrame},
	    
	    SectionRule{SectionType::IFrame, {"iframe"}, SubnameType::Parameter, ModuleType::Unknown, {},
                ContentType::None, ParameterType::Quoted, false},
        
		TreeRule{{Token::Type::Section, SectionType::IFrame}, handleIFrame},
		
		HtmlRule{Node::Type::IFrame, toHtmlNodeIFrame}
	}};
	
}

#endif // IFRAMERULESET_HPP
