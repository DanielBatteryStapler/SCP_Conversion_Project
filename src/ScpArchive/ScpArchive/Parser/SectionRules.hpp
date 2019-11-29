#ifndef SECTIONRULES_HPP
#define SECTIONRULES_HPP

#include "Parser.hpp"

namespace Parser{
    bool trySectionRule(const TokenRuleContext& context);
	TokenRuleResult doSectionRule(const TokenRuleContext& context);
}

#endif // SECTIONRULES_HPP
