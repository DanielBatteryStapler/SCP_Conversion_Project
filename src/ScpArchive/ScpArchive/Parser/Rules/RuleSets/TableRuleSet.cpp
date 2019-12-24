#include "TableRuleSet.hpp"

#include <sstream>

namespace Parser{
    std::string toStringTokenTableMarker(const TokenVariant& tok){
        const TableMarker& mark = std::get<TableMarker>(tok);
        std::stringstream ss;
        ss << "TableMarker:";
        switch(mark.type){
            case TableMarker::Type::Start:
                ss << "Start";
                break;
            case TableMarker::Type::Middle:
                ss << "Middle";
                break;
            case TableMarker::Type::End:
                ss << "End";
                break;
            case TableMarker::Type::StartEnd:
                ss << "StartEnd";
                break;
        }
        ss << ",";
        switch(mark.alignment){
            case TableMarker::AlignmentType::Default:
                ss << "Default";
                break;
            case TableMarker::AlignmentType::Header:
                ss << "Header";
                break;
            case TableMarker::AlignmentType::Left:
                ss << "Left";
                break;
            case TableMarker::AlignmentType::Right:
                ss << "Right";
                break;
            case TableMarker::AlignmentType::Center:
                ss << "Center";
                break;
        }
        ss << "," << mark.span;
        return ss.str();
    }
    
    std::string toStringNodeTable(const NodeVariant& nod){
        return "Table";
    }
    
    std::string toStringNodeTableRow(const NodeVariant& nod){
        return "TableRow";
    }
    
    std::string toStringNodeTableElement(const NodeVariant& nod){
        const TableElement& elem = std::get<TableElement>(nod);
        std::stringstream ss;
        ss << "TableElement:";
        switch(elem.alignment){
            case TableMarker::AlignmentType::Default:
                ss << "Default";
                break;
            case TableMarker::AlignmentType::Header:
                ss << "Header";
                break;
            case TableMarker::AlignmentType::Left:
                ss << "Left";
                break;
            case TableMarker::AlignmentType::Right:
                ss << "Right";
                break;
            case TableMarker::AlignmentType::Center:
                ss << "Center";
                break;
        }
        ss << "," << elem.span;
        return ss.str();
    }
    
	bool tryTableMarkerRule(const TokenRuleContext& context){
        return check(context.page, context.pagePos, "||");
	}
	
	TokenRuleResult doTableMarkerRule(const TokenRuleContext& context){
        TableMarker mark;
        mark.alignment = TableMarker::AlignmentType::Default;
        mark.span = 0;
        std::size_t pos = context.pagePos;
        while(pos < context.page.size() && check(context.page, pos, "||")){
            pos += 2;
            mark.span++;
        }
        if(check(context.page, pos, "~ ")){
            pos += 2;
            mark.alignment = TableMarker::AlignmentType::Header;
        }
        else if(check(context.page, pos, "< ")){
            pos += 2;
            mark.alignment = TableMarker::AlignmentType::Left;
        }
        else if(check(context.page, pos, "> ")){
            pos += 2;
            mark.alignment = TableMarker::AlignmentType::Right;
        }
        else if(check(context.page, pos, "= ")){
            pos += 2;
            mark.alignment = TableMarker::AlignmentType::Center;
        }
        
        if(pos >= context.page.size() || check(context.page, pos, "\n")){
            mark.type = TableMarker::Type::End;
        }
        else{
            mark.type = TableMarker::Type::Middle;
        }
        
        if(context.wasNewLine){
            if(mark.type == TableMarker::Type::End){
                mark.type = TableMarker::Type::StartEnd;
            }
            else{
                mark.type = TableMarker::Type::Start;
            }
        }
        
        TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{mark, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
	
    void handleTableMarker(TreeContext& context, const Token& token){
        const TableMarker& mark = std::get<TableMarker>(token.token);
        const auto makeTop = [&context](Node::Type type){
            while(context.stack.back().getType() != type){
                popStackWithCarry(context);
            }
        };
        
        switch(mark.type){
            case TableMarker::Type::Start:
                {
                    if(isInside(context, Node::Type::Table)){
                        makeTop(Node::Type::Table);
                    }
                    else{
                        makeDivPushable(context);
                        pushStack(context, Node{Table{}});
                    }
                    pushStack(context, Node{TableRow{}});
                    pushStack(context, Node{TableElement{mark.alignment, mark.span}});
                }
                break;
            case TableMarker::Type::StartEnd://kind of a weird edge case
                {
                    if(isInside(context, Node::Type::Table)){
                        makeTop(Node::Type::Table);
                    }
                    else{
                        makeDivPushable(context);
                        pushStack(context, Node{Table{}});
                    }
                    pushStack(context, Node{TableRow{}});
                    pushStack(context, Node{TableElement{mark.alignment, mark.span}});
                    popStack(context);
                    popStack(context);
                }
                break;
            case TableMarker::Type::Middle:
                {
                    if(isInside(context, Node::Type::TableElement)){
                        popSingle(context, Node::Type::TableElement);
                        pushStack(context, Node{TableElement{mark.alignment, mark.span}});
                    }
                    else{//if this is in the wrong place then just make it back into text
                        addAsText(context, Node{PlainText{token.source}});
                    }
                }
                break;
            case TableMarker::Type::End:
                {
                    if(isInside(context, Node::Type::TableRow)){
                        popSingle(context, Node::Type::TableRow);
                    }
                    else{//if this is in the wrong place then just make it back into text
                        addAsText(context, Node{PlainText{token.source}});
                    }
                }
                break;
        }
    }
	
	void toHtmlNodeTable(const HtmlContext& con, const Node& nod){
        con.out << "<table><tbody>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</tbody></table>"_AM;
	}
	
	void toHtmlNodeTableRow(const HtmlContext& con, const Node& nod){
        con.out << "<tr>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</tr>"_AM;
	}
	
	void toHtmlNodeTableElement(const HtmlContext& con, const Node& nod){
        const TableElement& elem = std::get<TableElement>(nod.node);
        switch(elem.alignment){
            case TableElement::AlignmentType::Default:
                con.out << "<td"_AM;
                break;
            case TableElement::AlignmentType::Header:
                con.out << "<th"_AM;
                break;
            case TableElement::AlignmentType::Left:
                con.out << "<td style='text-align:left;'"_AM;
                break;
            case TableElement::AlignmentType::Right:
                con.out << "<td style='text-align:right;'"_AM;
                break;
            case TableElement::AlignmentType::Center:
                con.out << "<td style='text-align:center;'"_AM;
                break;
        }
        if(elem.span == 1){
            con.out << ">"_AM;
        }
        else{
            con.out << " colspan='"_AM << std::to_string(elem.span) << "'>"_AM;
        }
        delegateNodeBranches(con, nod);
        if(elem.alignment == TableElement::AlignmentType::Header){
            con.out << "</th>"_AM;
        }
        else{
            con.out << "</td>"_AM;
        }
	}
}
