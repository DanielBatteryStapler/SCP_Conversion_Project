#ifndef CODERULESET_HPP
#define CODERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	
    void handleCode(TreeContext& context, const Token& token);
	
	const inline auto codeRuleSet = RuleSet{"Code", {
	    SectionRule{SectionType::Code, {"code"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Contain, ParameterType::Quoted, false},
        
		TreeRule{{Token::Type::Section, SectionType::Code}, handleCode}
	}};
	
}

#endif // CODERULESET_HPP
