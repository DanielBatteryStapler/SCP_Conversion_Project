#include "DivRuleSet.hpp"

namespace Parser{
    std::string toStringNodeDiv(const NodeVariant& nod){
        const Div& div = std::get<Div>(nod);
        std::string output = "Div:{";
        for(auto i = div.parameters.begin(); i != div.parameters.end(); i++){
            output += i->first + ": " + i->second + ", ";
        }
        output += "}";
        return output;
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
}
