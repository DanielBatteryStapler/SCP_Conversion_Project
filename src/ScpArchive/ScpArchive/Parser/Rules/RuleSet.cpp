#include "RuleSet.hpp"

#include "RuleSets/SpanRuleSet.hpp"
#include "RuleSets/SizeRuleSet.hpp"
#include "RuleSets/DivRuleSet.hpp"
#include "RuleSets/AnchorRuleSet.hpp"
#include "RuleSets/AlignRuleSet.hpp"
#include "RuleSets/CSSRuleSet.hpp"
#include "RuleSets/IncludeRuleSet.hpp"
#include "RuleSets/CodeRuleSet.hpp"
#include "RuleSets/CollapsibleRuleSet.hpp"

#include "RuleSets/SectionRuleSet.hpp"
#include "RuleSets/BasicTextRuleSet.hpp"
#include "RuleSets/LiteralTextRuleSet.hpp"
#include "RuleSets/EntityEscapeRuleSet.hpp"
#include "RuleSets/HyperLinkRuleSet.hpp"
#include "RuleSets/InlineFormatRuleSet.hpp"
#include "RuleSets/ListRuleSet.hpp"
#include "RuleSets/QuoteBoxRuleSet.hpp"
#include "RuleSets/DividerRuleSet.hpp"
#include "RuleSets/HeadingRuleSet.hpp"
#include "RuleSets/CommentRuleSet.hpp"
#include "RuleSets/NullRuleSet.hpp"

namespace Parser{
	const inline std::vector<RuleSet> ruleSets = {
	    spanRuleSet,
	    sizeRuleSet,
	    divRuleSet,
	    anchorRuleSet,
	    alignRuleSet,
	    cssRuleSet,
	    includeRuleSet,
	    codeRuleSet,
	    collapsibleRuleSet,
	    
	    commentRuleSet,
	    headingRuleSet,
	    dividerRuleSet,
	    quoteBoxRuleSet,
	    listRuleSet,
	    inlineFormatRuleSet,
	    tripleHyperLinkRuleSet,
		sectionRuleSet,
		hyperLinkRuleSet,
		entityEscapeRuleSet,
		literalTextRuleSet,
		basicTextRuleSet
		//, nullRuleSet
	};
	
	namespace{
		inline RuleType getType(const RuleVariant& var){
			return static_cast<RuleType>(var.index());
		}
		
		template<typename Type, RuleType type>
		inline std::vector<Type> getRules(){
			std::vector<Type> output;
			for(const RuleSet& ruleSet : ruleSets){
				for(const RuleVariant& ruleVariant : ruleSet.rules){
					if(getType(ruleVariant) == type){
						output.push_back(std::get<Type>(ruleVariant));
					}
				}
			}
			return output;
		}
	}
	
	std::vector<TokenPrintRule> getTokenPrintRules(){
        return getRules<TokenPrintRule, RuleType::TokenPrintRule>();
	}
	
	std::vector<NodePrintRule> getNodePrintRules(){
        return getRules<NodePrintRule, RuleType::NodePrintRule>();
	}
	
	std::vector<TokenRule> getTokenRules(){
		return getRules<TokenRule, RuleType::TokenRule>();
	}
	
	std::vector<SectionRule> getSectionRules(){
		return getRules<SectionRule, RuleType::SectionRule>();
	}
	
	std::vector<TreeRule> getTreeRules(){
		return getRules<TreeRule, RuleType::TreeRule>();
	}
	
	std::vector<HtmlRule> getHtmlRules(){
        return getRules<HtmlRule, RuleType::HtmlRule>();
	}
}
