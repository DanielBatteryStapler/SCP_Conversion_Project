#include "DivRuleSet.hpp"

namespace Parser{
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
