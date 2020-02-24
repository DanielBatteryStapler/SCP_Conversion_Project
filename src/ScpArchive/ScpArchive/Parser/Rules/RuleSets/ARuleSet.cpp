#include "ARuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeA(const NodeVariant& nod){
        const A& a = std::get<A>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = a.parameters.begin(); i != a.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
    }
    
    void handleA(TreeContext& context, const Token& token){
		handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{A{section.parameters}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::A)){
                popSingle(context, Node::Type::A);
            }
        });
    }
    
	void toHtmlNodeA(const HtmlContext& con, const Node& nod){
        const A& node = std::get<A>(nod.node);
        con.out << "<a"_AM;
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
        con.out << "</a>"_AM;
	}
}
