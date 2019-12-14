#include "AlignRuleSet.hpp"

namespace Parser{
    void handleAlign(TreeContext& context, const Token& token){
        handleSectionStartEnd(token,
        [&](const SectionStart& section){
            Align::Type type;
            if(section.typeString == "="){
                type = Align::Type::Center;
            }
            else if(section.typeString == "<"){
                type = Align::Type::Left;
            }
            else if(section.typeString == ">"){
                type = Align::Type::Right;
            }
            else if(section.typeString == "=="){
                type = Align::Type::Justify;
            }
            makeDivPushable(context);
            pushStack(context, Node{Align{type}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Align)){//not actually correct, this should keep track of what kind of align it is talking about
                //but who's nesting different kind of aligns anyways? this work just fine, I'm sure...
                popSingle(context, Node::Type::Align);
            }
        });
    }
}
