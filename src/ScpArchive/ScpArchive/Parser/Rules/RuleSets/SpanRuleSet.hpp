#ifndef SPANRULESET_HPP
#define SPANRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    
    void handleSpan(TreeContext& context, const Token& token);
	
	const inline auto spanRuleSet = RuleSet{"Span", {
		SectionRule{SectionType::Span, {"span"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::Quoted, true},
        
        TreeRule{{Token::Type::Section, SectionType::Span}, handleSpan}
	}};
	
}

#endif // SPANRULESET_HPP
