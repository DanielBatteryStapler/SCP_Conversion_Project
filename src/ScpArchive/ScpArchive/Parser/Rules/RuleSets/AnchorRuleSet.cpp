#include "AnchorRuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeAnchor(const NodeVariant& nod){
        const Anchor& anchor = std::get<Anchor>(nod);
        return anchor.name;
	}
    
    void handleAnchor(TreeContext& context, const Token& token){
		const Section& section = std::get<Section>(token.token);
		addAsText(context, Node{Anchor{section.mainParameter}});
    }
    
    void toHtmlNodeAnchor(const HtmlContext& con, const Node& nod){
        const Anchor& node = std::get<Anchor>(nod.node);
        con.out << "<a name='"_AM << node.name << "'></a>"_AM;
    }
}
