#ifndef INCLUDERULESET_HPP
#define INCLUDERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    
    void handleInclude(TreeContext& context, const Token& token);
	
	const inline auto includeRuleSet = RuleSet{"Include", {
		SectionRule{SectionType::Include, {"include"}, SubnameType::Parameter, ModuleType::Unknown, {},
            ContentType::None, ParameterType::Lined, false},
		
        TreeRule{{Token::Type::Section, SectionType::Include}, handleInclude}
	}};
	
}

#endif // INCLUDERULESET_HPP
