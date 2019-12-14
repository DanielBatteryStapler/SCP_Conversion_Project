#include "NullRuleSet.hpp"

namespace Parser{
    bool tryNullRule(const TokenRuleContext& context){
		return true;
	}
	
	TokenRuleResult doNullRule(const TokenRuleContext& context){
		TokenRuleResult result;
		result.newPos = context.pagePos + 1;
		return result;
	}
}
