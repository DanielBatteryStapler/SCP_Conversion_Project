#ifndef SCPCONVERSIONPROJECTINFOBOXRULESET_HPP
#define SCPCONVERSIONPROJECTINFOBOXRULESET_HPP

#include "../RuleSet.hpp"

//The stupidly long name is on purpose, I want to make it obvious that this isn't something added by wikidot;
//this is honest to goodness a completely new feature that would never work with the old wiki.
//I also want to make it impossible to accidentally use in some other code without realizing that this isn't a real formatting element.

namespace Parser{
	nlohmann::json printNodeSCPConversionProjectInfoBox(const NodeVariant& nod);
	
    void handleSCPConversionProjectInfoBox(TreeContext& context, const Token& token);
	
	void toHtmlNodeSCPConversionProjectInfoBox(const HtmlContext& con, const Node& nod);
	
	const inline auto scpConversionProjectInfoBoxRuleSet = RuleSet{"SCPConversionProjectInfoBox", {
		NodePrintRule{Node::Type::SCPConversionProjectInfoBox, printNodeSCPConversionProjectInfoBox},
		
		SectionRule{SectionType::SCPConversionProjectInfoBox, {"scp_conversion_project_info_box"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::None, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::SCPConversionProjectInfoBox}, handleSCPConversionProjectInfoBox},
        
        HtmlRule{Node::Type::SCPConversionProjectInfoBox, toHtmlNodeSCPConversionProjectInfoBox}
	}};
	
}

#endif // SCPCONVERSIONPROJECTINFOBOXRULESET_HPP
