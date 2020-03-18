#include "FootNoteRuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeFootNote(const NodeVariant& nod){
        return std::get<FootNote>(nod).number;
	}
	
	nlohmann::json printNodeFootNoteBlock(const NodeVariant& nod){
        return std::get<FootNoteBlock>(nod).title;
	}
	
    void handleFootNote(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{FootNote{}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::FootNote)){
                popSingle(context, Node::Type::FootNote);
            }
        });
    }
    
    void handleFootNoteBlock(TreeContext& context, const Token& token){
        const Section& section = std::get<Section>(token.token);
        FootNoteBlock block;
        if(section.parameters.find("title") != section.parameters.end()){
            block.title = section.parameters.find("title")->second;
        }
        else{
            block.title = "Footnotes";
        }
        addAsDiv(context, Node{block});
    }
	
	void postTreeRuleFootNotes(TreeContext& context){
        std::vector<Node> footnotes;
        bool foundBlock = false;
        travelPageTreeNodes(context.stack.back(), [&foundBlock, &footnotes](Node& node)->bool{
            Node::Type type = node.getType();
            if(type == Node::Type::TableOfContents){
                return false;
            }
            else if(type == Node::Type::FootNoteBlock){
				foundBlock = true;
                return false;
            }
            else if(type == Node::Type::FootNote){
                FootNote& footnote = std::get<FootNote>(node.node);
                footnote.number = footnotes.size() + 1;
                footnotes.push_back(node);
            }
            return true;
        });
        if(!foundBlock && footnotes.size() > 0){
            addAsDiv(context, Node{FootNoteBlock{"Footnotes"}});
        }
        travelPageTreeNodes(context.stack.back(), [&footnotes](Node& node)->bool{
            Node::Type type = node.getType();
            if(type == Node::Type::FootNoteBlock){
                node.branches = footnotes;
                return false;
            }
            return true;
        });
	}
	
	void toHtmlNodeFootNote(const HtmlContext& con, const Node& nod){
        const FootNote& footnote = std::get<FootNote>(nod.node);
        con.out << "<a class='footNote' id='footNote"_AM << std::to_string(footnote.number) << "' href='#footNoteBlock"_AM << std::to_string(footnote.number) << "'>"_AM
        << "<sup>["_AM << std::to_string(footnote.number) << "]</sup></a>"_AM
        << "<div class='footNoteDropDownAnchor'><div class='footNoteDropDown'>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</div></div>"_AM;
       
	}
	
	void toHtmlNodeFootNoteBlock(const HtmlContext& con, const Node& nod){
        if(nod.branches.size() > 0){
            const FootNoteBlock& block = std::get<FootNoteBlock>(nod.node);
            con.out << "<h2>"_AM << block.title << "</h2>"_AM
            << "<ol class='footNoteBlock'>"_AM;
            std::size_t i = 1;
            for(const Node& branch : nod.branches){
                con.out << "<li><a class='footNoteBlockLabel' id='footNoteBlock"_AM << std::to_string(i) << "' href='#footNote"_AM << std::to_string(i) << "'>"_AM << std::to_string(i) << ".</a> "_AM;
                delegateNodeBranches(con, branch);
                con.out << "</li>"_AM;
                i++;
            }
            con.out << "</ol>"_AM;
        }
	}
}
