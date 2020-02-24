#ifndef RATERULESET_HPP
#define RATERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeRate(const NodeVariant& nod);
	
    void handleRate(TreeContext& context, const Token& token);
	
	void toHtmlNodeRate(const HtmlContext& con, const Node& nod);
	
	const inline auto rateRuleSet = RuleSet{"Rate", {
		NodePrintRule{Node::Type::Rate, printNodeRate},
		
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::Rate, {"rate"},
                ContentType::None, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::Rate}, handleRate},
        
        HtmlRule{Node::Type::Rate, toHtmlNodeRate}
	}};
	
}

#endif // RATERULESET_HPP
