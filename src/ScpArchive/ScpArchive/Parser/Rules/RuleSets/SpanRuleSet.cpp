#include "SpanRuleSet.hpp"

#include <sstream>

namespace Parser{
    std::string toStringNodeSpan(const NodeVariant& nod){
        const Span& span = std::get<Span>(nod);
        std::stringstream ss;
        ss << "Span:{";
        for(auto i = span.parameters.begin(); i != span.parameters.end(); i++){
            ss << i->first << ": " << i->second << ", ";
        }
        ss << "}";
        return ss.str();
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
}
