#ifndef IFTAGSRULESET_HPP
#define IFTAGSRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    
    void handleIftags(TreeContext& context, const Token& token);
	
	const inline auto iftagsRuleSet = RuleSet{"Iftags", {
		SectionRule{SectionType::Iftags, {"iftags"}, SubnameType::None, ModuleType::Unknown, {},
            ContentType::Surround, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Iftags}, handleIftags}
	}};
	
}

#endif // IFTAGSRULESET_HPP
