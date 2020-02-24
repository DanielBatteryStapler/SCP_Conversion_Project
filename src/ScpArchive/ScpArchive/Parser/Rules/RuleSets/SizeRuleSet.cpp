#include "SizeRuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeSize(const NodeVariant& nod){
        const Size& size = std::get<Size>(nod);
		return size.size;
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
    
	void toHtmlNodeSize(const HtmlContext& con, const Node& nod){
        const Size& node = std::get<Size>(nod.node);
        con.out << "<span style='font-size:"_AM << node.size << "'>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</span>"_AM;
	}
}
