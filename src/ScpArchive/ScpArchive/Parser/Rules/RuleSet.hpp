#ifndef RULESET_HPP
#define RULESET_HPP

#include "../Parser.hpp"
#include "../Treer.hpp"

#include "RuleSetUtil.hpp"

namespace Parser{
	struct TokenRule{
		std::function<bool(const TokenRuleContext&)> tryRule;
		std::function<TokenRuleResult(const TokenRuleContext&)> doRule;
	};
	
	enum class SubnameType{None, Parameter, Module};
	enum class ContentType{None, Surround, Contain};
	enum class ParameterType{None, Quoted, Lined};
	
	struct SectionRule{
		SectionType type;
		std::vector<std::string> matchingNames;
		SubnameType subnameType;
		ModuleType moduleType;
		std::vector<std::string> matchingModules;
		ContentType contentType;
		ParameterType parameterType;
		bool allowInline;
	};
	
	struct TreeRule{
        struct TreeRuleType{
            Token::Type mainType;
            SectionType sectionType = SectionType::Unknown;
            ModuleType moduleType = ModuleType::Unknown;
        };
        TreeRuleType type;
        std::function<void(TreeContext&, const Token&)> handleRule;
    };
	
	using RuleVariant = std::variant<TokenRule, SectionRule, TreeRule>;
	enum class RuleType{TokenRule = 0, SectionRule, TreeRule};
	
	struct RuleSet{
		std::string name;
		std::vector<RuleVariant> rules;
	};
	
	std::vector<TokenRule> getTokenRules();
	std::vector<SectionRule> getSectionRules();
	std::vector<TreeRule> getTreeRules();
	
}

#endif // RULESET_HPP
