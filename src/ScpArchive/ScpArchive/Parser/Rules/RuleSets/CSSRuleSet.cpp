#include "CSSRuleSet.hpp"

namespace Parser{
    void handleCSS(TreeContext& context, const Token& token){
		const SectionComplete& sectionComplete = std::get<SectionComplete>(token.token);
		//just add the css to the cssData, don't even touch the tree
		context.cssData.push_back(CSS{sectionComplete.contents});
    }
}
