#ifndef HTMLRULESET_HPP
#define HTMLRULESET_HPP


#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeHTML(const NodeVariant& nod);
	
    void handleHTML(TreeContext& context, const Token& token);
	
	void toHtmlNodeHTML(const HtmlContext& con, const Node& nod);
	
	const inline auto htmlRuleSet = RuleSet{"HTML", {
	    NodePrintRule{Node::Type::HTML, printNodeHTML},
	    
	    SectionRule{SectionType::HTML, {"html", "embed", "embedvideo", "embedaudio"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Contain, ParameterType::None, false},
        
		TreeRule{{Token::Type::Section, SectionType::HTML}, handleHTML},
		
		HtmlRule{Node::Type::HTML, toHtmlNodeHTML}
	}};
	
}

#endif // HTMLRULESET_HPP
