#ifndef CSSRULESET_HPP
#define CSSRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
    void handleCSS(TreeContext& context, const Token& token);
	
	const inline auto cssRuleSet = RuleSet{"CSS", {
        SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::CSS, {"css"},
            ContentType::Contain, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::CSS}, handleCSS}
	}};
	
}

#endif // CSSRULESET_HPP
