#ifndef FOOTNOTERULESET_HPP
#define FOOTNOTERULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeFootNote(const NodeVariant& nod);
	nlohmann::json printNodeFootNoteBlock(const NodeVariant& nod);
	
    void handleFootNote(TreeContext& context, const Token& token);
    void handleFootNoteBlock(TreeContext& context, const Token& token);
	
	void postTreeRuleFootNotes(TreeContext& context);
	
	void toHtmlNodeFootNote(const HtmlContext& con, const Node& nod);
	void toHtmlNodeFootNoteBlock(const HtmlContext& con, const Node& nod);
	
	const inline auto footNoteRuleSet = RuleSet{"FootNote", {
		NodePrintRule{Node::Type::FootNote, printNodeFootNote},
		NodePrintRule{Node::Type::FootNoteBlock, printNodeFootNoteBlock},
		
		SectionRule{SectionType::FootNote, {"footnote"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::Surround, ParameterType::None, true},
        SectionRule{SectionType::FootNoteBlock, {"footnoteblock"}, SubnameType::None, ModuleType::Unknown, {},
                ContentType::None, ParameterType::Quoted, false},
		
        TreeRule{{Token::Type::Section, SectionType::FootNote}, handleFootNote},
        TreeRule{{Token::Type::Section, SectionType::FootNoteBlock}, handleFootNoteBlock},
        
        PostTreeRule{postTreeRuleFootNotes},
        
        HtmlRule{Node::Type::FootNote, toHtmlNodeFootNote},
        HtmlRule{Node::Type::FootNoteBlock, toHtmlNodeFootNoteBlock}
	}};
	
}

#endif // FOOTNOTERULESET_HPP
