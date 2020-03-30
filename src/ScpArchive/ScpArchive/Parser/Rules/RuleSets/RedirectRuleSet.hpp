#ifndef REDIRECTRULESET_HPP
#define REDIRECTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    void handleRedirect(TreeContext& context, const Token& token);
	
	const inline auto redirectRuleSet = RuleSet{"Redirect", {
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::Redirect, {"redirect"},
                ContentType::None, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::Redirect}, handleRedirect},
	}};
	
}

#endif // REDIRECTRULESET_HPP
