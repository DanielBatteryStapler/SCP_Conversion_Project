#ifndef USERRULESET_HPP
#define USERRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeUser(const NodeVariant& nod);
	
    void handleUser(TreeContext& context, const Token& token);
	
	void toHtmlNodeUser(const HtmlContext& con, const Node& nod);
	
	const inline auto userRuleSet = RuleSet{"User", {
		NodePrintRule{Node::Type::User, printNodeUser},
		
		SectionRule{SectionType::User, {"user", "*user"}, SubnameType::Parameter, ModuleType::Unknown, {},
                ContentType::None, ParameterType::None, true},
		
        TreeRule{{Token::Type::Section, SectionType::User}, handleUser},
        
        HtmlRule{Node::Type::User, toHtmlNodeUser}
	}};
	
}

#endif // USERRULESET_HPP
