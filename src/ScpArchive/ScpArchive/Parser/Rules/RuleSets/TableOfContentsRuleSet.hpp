#ifndef TABLEOFCONTENTSRULESET_HPP
#define TABLEOFCONTENTSRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeTableOfContents(const NodeVariant& nod);
	
    void handleTableOfContents(TreeContext& context, const Token& token);
	
	void postTreeRuleTableOfContents(TreeContext& context);
	
	void toHtmlNodeTableOfContents(const HtmlContext& con, const Node& nod);
	
	const inline auto tableOfContentsRuleSet = RuleSet{"TableOfContents", {
		NodePrintRule{Node::Type::TableOfContents, printNodeTableOfContents},
		
		SectionRule{SectionType::TableOfContents, {"toc", "f<toc", "f>toc"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::None, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::TableOfContents}, handleTableOfContents},
        
        PostTreeRule{postTreeRuleTableOfContents},
        
        HtmlRule{Node::Type::TableOfContents, toHtmlNodeTableOfContents}
	}};
	
}


#endif // TABLEOFCONTENTSRULESET_HPP
