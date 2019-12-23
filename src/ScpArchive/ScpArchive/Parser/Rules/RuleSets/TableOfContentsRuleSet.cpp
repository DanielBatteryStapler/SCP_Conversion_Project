#include "TableOfContentsRuleSet.hpp"

namespace Parser{
    std::string toStringNodeTableOfContents(const NodeVariant& nod){
        const TableOfContents& toc = std::get<TableOfContents>(nod);
        switch(toc.alignment){
            case TableOfContents::AlignmentType::Default:
                return "TableOfContents:Default";
            case TableOfContents::AlignmentType::FloatLeft:
                return "TableOfContents:FloatLeft";
            case TableOfContents::AlignmentType::FloatRight:
                return "TableOfContents:FloatRight";
        }
    }

    void handleTableOfContents(TreeContext& context, const Token& token){
        const Section& section = std::get<Section>(token.token);
        
        TableOfContents toc;
        if(check(section.typeString, 0, "f<")){
            toc.alignment = TableOfContents::AlignmentType::FloatLeft;
        }
        else if(check(section.typeString, 0, "f>")){
            toc.alignment = TableOfContents::AlignmentType::FloatRight;
        }
        else{
            toc.alignment = TableOfContents::AlignmentType::Default;
        }
        
        addAsDiv(context, Node{toc});
    }
    
	void postTreeRuleTableOfContents(TreeContext& context){
        std::vector<Node> headings;
        travelPageTreeNodes(context.stack.back(), [&headings](Node& node)->bool{
            Node::Type type = node.getType();
            if(type == Node::Type::TableOfContents || type == Node::Type::FootNoteBlock){
                return false;
            }
            if(type == Node::Type::Heading){
                Heading& heading = std::get<Heading>(node.node);
                if(heading.hidden == false){
                    heading.tocNumber = headings.size();
                    headings.push_back(node);
                }
            }
            return true;
        });
        travelPageTreeNodes(context.stack.back(), [&headings](Node& node)->bool{
            Node::Type type = node.getType();
            if(type == Node::Type::TableOfContents){
                node.branches = headings;
                return false;
            }
            return true;
        });
	}
    
    void toHtmlNodeTableOfContents(const HtmlContext& con, const Node& nod){
        const TableOfContents& toc = std::get<TableOfContents>(nod.node);
        con.out << "<div id='toc'"_AM;
        switch(toc.alignment){
            case TableOfContents::AlignmentType::Default:
                con.out << ">"_AM;
                break;
            case TableOfContents::AlignmentType::FloatLeft:
                con.out << " style='float:left;'>"_AM;
                break;
            case TableOfContents::AlignmentType::FloatRight:
                con.out << " style='float:right;'>"_AM;
                break;
        }
        
        con.out << "<div style='text-align:center;font-weight:bold;'>Table of Contents</div>"_AM;
        for(const Node& headingNode : nod.branches){
            const Heading& heading = std::get<Heading>(headingNode.node);
            con.out << "<div style='margin-left:"_AM << std::to_string(heading.degree) << "em;'>"_AM
            << "<a href='#toc"_AM << std::to_string(heading.tocNumber) << "'>"_AM;
            delegateNodeBranches(con, headingNode);
            con.out << "</a></div>"_AM;
        }
        con.out << "</div>"_AM;
    }
}
