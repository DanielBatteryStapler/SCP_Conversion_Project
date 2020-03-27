#ifndef RULESET_HPP
#define RULESET_HPP

#include <nlohmann/json.hpp>

#include "../Parser.hpp"
#include "../Treer.hpp"
#include "../HTMLConverter.hpp"

#include "RuleSetUtil.hpp"

namespace Parser{
	struct TokenPrintRule{
        Token::Type type;
        std::function<nlohmann::json(const TokenVariant&)> print;
        
        std::string parentRuleSet;
	};
	
	struct NodePrintRule{
        Node::Type type;
        std::function<nlohmann::json(const NodeVariant&)> print;
        
        std::string parentRuleSet;
	};
	
	struct TokenRule{
		std::function<bool(const TokenRuleContext&)> tryRule;
		std::function<TokenRuleResult(const TokenRuleContext&)> doRule;
		
		std::string parentRuleSet;
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
		
		std::string parentRuleSet;
	};
	
	struct TreeRule{
        struct TreeRuleType{
            Token::Type mainType;
            SectionType sectionType = SectionType::Unknown;
            ModuleType moduleType = ModuleType::Unknown;
        };
        TreeRuleType type;
        std::function<void(TreeContext&, const Token&)> handleRule;
		
		std::string parentRuleSet;
    };
	
	struct PostTreeRule{
        std::function<void(TreeContext&)> rule;
		
		std::string parentRuleSet;
	};
	
	struct HtmlRule{
        Node::Type type;
        std::function<void(const HtmlContext&, const Node&)> handleRule;
		
		std::string parentRuleSet;
	};
	
	using RuleVariant = std::variant<TokenPrintRule, NodePrintRule, TokenRule, SectionRule, TreeRule, PostTreeRule, HtmlRule>;
	enum class RuleType{TokenPrintRule = 0, NodePrintRule, TokenRule, SectionRule, TreeRule, PostTreeRule, HtmlRule};
	
	struct RuleSet{
		std::string name;
		std::vector<RuleVariant> rules;
	};
	
	std::vector<TokenPrintRule> getTokenPrintRules();
	std::vector<NodePrintRule> getNodePrintRules();
	std::vector<TokenRule> getTokenRules();
	std::vector<SectionRule> getSectionRules();
	std::vector<TreeRule> getTreeRules();
	std::vector<PostTreeRule> getPostTreeRules();
	std::vector<HtmlRule> getHtmlRules();
	
	void printFullRuleSetList();
}

#endif // RULESET_HPP
