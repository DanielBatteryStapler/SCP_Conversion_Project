#include "DivRuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeDiv(const NodeVariant& nod){
        const Div& div = std::get<Div>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = div.parameters.begin(); i != div.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
    }
    
    void handleDiv(TreeContext& context, const Token& token){
		handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeDivPushable(context);
            pushStack(context, Node{Div{section.parameters}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Div)){
                popSingle(context, Node::Type::Div);
            }
        });
    }
    
	void toHtmlNodeDiv(const HtmlContext& con, const Node& nod){
        const Div& node = std::get<Div>(nod.node);
        con.out << "<div"_AM;
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
        con.out << "</div>"_AM;
	}
}
