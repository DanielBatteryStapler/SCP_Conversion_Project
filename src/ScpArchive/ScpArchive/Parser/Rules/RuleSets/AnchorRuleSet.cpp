#include "AnchorRuleSet.hpp"

namespace Parser{
	std::string toStringNodeAnchor(const NodeVariant& nod){
        const Anchor& anchor = std::get<Anchor>(nod);
        return "Anchor:" + anchor.name;
	}
    
    void handleAnchor(TreeContext& context, const Token& token){
		const Section& section = std::get<Section>(token.token);
		addAsText(context, Node{Anchor{section.mainParameter}});
    }
}
