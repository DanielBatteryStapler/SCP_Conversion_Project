#ifndef LISTRULESET_HPP
#define LISTRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
    std::string toStringTokenListPrefix(const TokenVariant& tok);
    std::string toStringNodeList(const NodeVariant& nod);
    std::string toStringNodeListElement(const NodeVariant& nod);
	
	bool tryListPrefixRule(const TokenRuleContext& context);
	TokenRuleResult doListPrefixRule(const TokenRuleContext& context);
	
	const inline auto listRuleSet = RuleSet{"List", {
	    TokenPrintRule{Token::Type::ListPrefix, toStringTokenListPrefix},
	    NodePrintRule{Node::Type::List, toStringNodeList},
	    NodePrintRule{Node::Type::ListElement, toStringNodeListElement},
	    
		TokenRule{tryListPrefixRule, doListPrefixRule}
	}};
	
}

#endif // LISTRULESET_HPP
