#include "SpanRuleSet.hpp"

#include <sstream>

namespace Parser{
    nlohmann::json printNodeSpan(const NodeVariant& nod){
        const Span& span = std::get<Span>(nod);
        nlohmann::json out = nlohmann::json::object();
        for(auto i = span.parameters.begin(); i != span.parameters.end(); i++){
            out[i->first] = i->second;
        }
        return out;
    }
    
    void handleSpan(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{Span{section.parameters}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Span)){
                popSingle(context, Node::Type::Span);
            }
        });
    }
    
	void toHtmlNodeSpan(const HtmlContext& con, const Node& nod){
        const Span& node = std::get<Span>(nod.node);
        con.out << "<span"_AM;
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
        con.out << "</span>"_AM;
	}
}
