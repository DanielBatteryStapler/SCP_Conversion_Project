#ifndef CODERULESET_HPP
#define CODERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
	std::string toStringNodeCode(const NodeVariant& nod);
	
    void handleCode(TreeContext& context, const Token& token);
	
	void toHtmlNodeCode(const HtmlContext& con, const Node& nod);
	
	const inline auto codeRuleSet = RuleSet{"Code", {
	    NodePrintRule{Node::Type::Code, toStringNodeCode},
	    
	    SectionRule{SectionType::Code, {"code"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Contain, ParameterType::Quoted, false},
        
		TreeRule{{Token::Type::Section, SectionType::Code}, handleCode},
		
		HtmlRule{Node::Type::Code, toHtmlNodeCode}
	}};
	
}

#endif // CODERULESET_HPP
