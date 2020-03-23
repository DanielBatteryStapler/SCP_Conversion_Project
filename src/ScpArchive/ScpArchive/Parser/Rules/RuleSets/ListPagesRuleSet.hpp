#ifndef LISTPAGESRULESET_HPP
#define LISTPAGESRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    
    void handleListPages(TreeContext& context, const Token& token);
	
	const inline auto listPagesRuleSet = RuleSet{"ListPages", {
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::ListPages, {"listpages"},
            ContentType::Contain, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::ListPages}, handleListPages}
	}};
	
}

#endif // LISTPAGESRULESET_HPP
