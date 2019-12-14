#include "SizeRuleSet.hpp"

namespace Parser{
    void handleSize(TreeContext& context, const Token& token){
        handleSectionStartEnd(token,
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{Size{section.mainParameter}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Size)){
                popSingle(context, Node::Type::Size);
            }
        });
    }
}
