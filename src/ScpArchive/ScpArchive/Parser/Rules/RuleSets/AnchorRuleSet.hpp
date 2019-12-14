#ifndef ANCHORRULESET_HPP
#define ANCHORRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
    void handleAnchor(TreeContext& context, const Token& token);
	
	const inline auto anchorRuleSet = RuleSet{"Anchor", {
		SectionRule{SectionType::Anchor, {"#"}, SubnameType::Parameter, ModuleType::Unknown, {},
                ContentType::None, ParameterType::None, true},
		
        TreeRule{{Token::Type::Section, SectionType::Anchor}, handleAnchor}
	}};
	
}

#endif // ANCHORRULESET_HPP
