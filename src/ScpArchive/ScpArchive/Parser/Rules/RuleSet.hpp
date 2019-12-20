#ifndef RULESET_HPP
#define RULESET_HPP

#include "../Parser.hpp"
#include "../Treer.hpp"
#include "../HTMLConverter.hpp"

#include "RuleSetUtil.hpp"

namespace Parser{
	struct TokenPrintRule{
        Token::Type type;
        std::function<std::string(const TokenVariant&)> toString;
	};
	
	struct NodePrintRule{
        Node::Type type;
        std::function<std::string(const NodeVariant&)> toString;
	};
	
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
	
	struct HtmlRule{
        Node::Type type;
        std::function<void(const HtmlContext&, const Node&)> handleRule;
	};
	
	using RuleVariant = std::variant<TokenPrintRule, NodePrintRule, TokenRule, SectionRule, TreeRule, HtmlRule>;
	enum class RuleType{TokenPrintRule = 0, NodePrintRule, TokenRule, SectionRule, TreeRule, HtmlRule};
	
	struct RuleSet{
		std::string name;
		std::vector<RuleVariant> rules;
	};
	
	std::vector<TokenPrintRule> getTokenPrintRules();
	std::vector<NodePrintRule> getNodePrintRules();
	std::vector<TokenRule> getTokenRules();
	std::vector<SectionRule> getSectionRules();
	std::vector<TreeRule> getTreeRules();
	std::vector<HtmlRule> getHtmlRules();
	
	void printFullRuleSetList();
}

#endif // RULESET_HPP
