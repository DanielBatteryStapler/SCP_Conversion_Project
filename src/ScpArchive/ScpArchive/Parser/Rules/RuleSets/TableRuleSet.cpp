#include "TableRuleSet.hpp"

#include <sstream>

namespace Parser{
    nlohmann::json printTokenTableMarker(const TokenVariant& tok){
        const TableMarker& mark = std::get<TableMarker>(tok);
        nlohmann::json out;
        out["span"] = mark.span;
        switch(mark.type){
            case TableMarker::Type::Start:
                out["type"] = "Start";
                break;
            case TableMarker::Type::Middle:
                out["type"] = "Middle";
                break;
            case TableMarker::Type::End:
                out["type"] = "End";
                break;
            case TableMarker::Type::StartEnd:
                out["type"] = "StartEnd";
                break;
			default:
                out["type"] = "Unknown";
                break;
        }
        switch(mark.alignment){
            case TableMarker::AlignmentType::Default:
                out["alignment"] = "Default";
                break;
            case TableMarker::AlignmentType::Header:
                out["alignment"] = "Header";
                break;
            case TableMarker::AlignmentType::Left:
                out["alignment"] = "Left";
                break;
            case TableMarker::AlignmentType::Right:
                out["alignment"] = "Right";
                break;
            case TableMarker::AlignmentType::Center:
                out["alignment"] = "Center";
                break;
			default:
                out["alignment"] = "Unknown";
                break;
        }
        return out;
    }
    
    nlohmann::json printNodeTable(const NodeVariant& nod){
        return {};
    }
    
    nlohmann::json printNodeTableRow(const NodeVariant& nod){
        return {};
    }
    
    nlohmann::json printNodeTableElement(const NodeVariant& nod){
        const TableElement& elem = std::get<TableElement>(nod);
        nlohmann::json out;
        out["span"] = elem.span;
        switch(elem.alignment){
            case TableMarker::AlignmentType::Default:
                out["alignment"] = "Default";
                break;
            case TableMarker::AlignmentType::Header:
                out["alignment"] = "Header";
                break;
            case TableMarker::AlignmentType::Left:
                out["alignment"] = "Left";
                break;
            case TableMarker::AlignmentType::Right:
                out["alignment"] = "Right";
                break;
            case TableMarker::AlignmentType::Center:
                out["alignment"] = "Center";
                break;
			default:
				out["alignment"] = "Unknown";
				break;
        }
        return out;
    }
    
    nlohmann::json printNodeAdvTable(const NodeVariant& nod){
        const AdvTable& table = std::get<AdvTable>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = table.parameters.begin(); i != table.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
    }
    
    nlohmann::json printNodeAdvTableRow(const NodeVariant& nod){
        const AdvTableRow& table = std::get<AdvTableRow>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = table.parameters.begin(); i != table.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
    }
    
    nlohmann::json printNodeAdvTableElement(const NodeVariant& nod){
        const AdvTableElement& table = std::get<AdvTableElement>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = table.parameters.begin(); i != table.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
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
        
        switch(mark.type){
            case TableMarker::Type::Start:
                {
                    if(isInside(context, Node::Type::Table)){
                        makeTop(context, Node::Type::Table);
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
                        makeTop(context, Node::Type::Table);
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
	
	void handleAdvTable(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeDivPushable(context);
            pushStack(context, Node{AdvTable{section.parameters}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::AdvTable)){
                popSingle(context, Node::Type::AdvTable);
            }
        });
    }
    
	void handleAdvTableRow(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            if(isInside(context, Node::Type::AdvTable)){
                makeTop(context, Node::Type::AdvTable);
                pushStack(context, Node{AdvTableRow{section.parameters}});
            }
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::AdvTableRow)){
                popSingle(context, Node::Type::AdvTableRow);
            }
        });
	}
	
	void handleAdvTableElement(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            if(isInside(context, Node::Type::AdvTableRow)){
                makeTop(context, Node::Type::AdvTableRow);
                pushStack(context, Node{AdvTableElement{section.typeString == "hcell", section.parameters}});
            }
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::AdvTableElement)){
                popSingle(context, Node::Type::AdvTableElement);
            }
        });
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
	
	void toHtmlNodeAdvTable(const HtmlContext& con, const Node& nod){
        const AdvTable& node = std::get<AdvTable>(nod.node);
        con.out << "<table"_AM;
        for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << "><tbody>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</tbody></table>"_AM;
	}
	
	void toHtmlNodeAdvTableRow(const HtmlContext& con, const Node& nod){
        const AdvTableRow& node = std::get<AdvTableRow>(nod.node);
        con.out << "<tr"_AM;
        for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << ">"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</tr>"_AM;
	}
	
	void toHtmlNodeAdvTableElement(const HtmlContext& con, const Node& nod){
        const AdvTableElement& node = std::get<AdvTableElement>(nod.node);
        if(node.isHeader){
            con.out << "<th"_AM;
        }
        else{
            con.out << "<td"_AM;
        }
        for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << ">"_AM;
        delegateNodeBranches(con, nod);
        if(node.isHeader){
            con.out << "</th>"_AM;
        }
        else{
            con.out << "</td>"_AM;
        }
	}
}
