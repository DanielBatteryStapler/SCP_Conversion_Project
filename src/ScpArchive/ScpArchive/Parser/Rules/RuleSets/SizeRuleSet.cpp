#include "SizeRuleSet.hpp"

namespace Parser{
    std::string toStringNodeSize(const NodeVariant& nod){
        const Size& size = std::get<Size>(nod);
		return "Size:\"" + size.size + "\"";
    }
    
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
