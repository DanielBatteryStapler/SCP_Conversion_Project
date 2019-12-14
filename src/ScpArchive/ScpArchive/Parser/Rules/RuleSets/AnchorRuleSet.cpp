#include "AnchorRuleSet.hpp"

namespace Parser{
    void handleAnchor(TreeContext& context, const Token& token){
		const Section& section = std::get<Section>(token.token);
		addAsText(context, Node{Anchor{section.mainParameter}});
    }
}
