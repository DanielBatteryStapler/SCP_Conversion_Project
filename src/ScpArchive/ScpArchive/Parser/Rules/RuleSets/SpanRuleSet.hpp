#ifndef SPANRULESET_HPP
#define SPANRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringNodeSpan(const NodeVariant& nod);
    
    void handleSpan(TreeContext& context, const Token& token);
	
	void toHtmlNodeSpan(const HtmlContext& con, const Node& nod);
	
	const inline auto spanRuleSet = RuleSet{"Span", {
	    NodePrintRule{Node::Type::Span, toStringNodeSpan},
	    
		SectionRule{SectionType::Span, {"span", "span_"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, true},
        
        TreeRule{{Token::Type::Section, SectionType::Span}, handleSpan},
        
        HtmlRule{Node::Type::Span, toHtmlNodeSpan}
	}};
	
}

#endif // SPANRULESET_HPP
