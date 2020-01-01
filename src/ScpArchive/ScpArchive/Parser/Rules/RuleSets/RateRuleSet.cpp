#include "RateRuleSet.hpp"

namespace Parser{
	std::string toStringNodeRate(const NodeVariant& nod){
        const Rate& rate = std::get<Rate>(nod);
        return "Rate:" + std::to_string(rate.rating);
	}
	
    void handleRate(TreeContext& context, const Token& token){
        addAsDiv(context, Node{Rate{context.parameters.page.rating}});
    }
	
	void toHtmlNodeRate(const HtmlContext& con, const Node& nod){
        const Rate& rate = std::get<Rate>(nod.node);
        con.out
        << "<div class='page-rate-widget-box'>"_AM
        << "<span class='rate-points'>"_AM
        << "Rating: " << ((rate.rating > 0)?"+":"") << std::to_string(rate.rating)
        << "</span>"_AM
        << "</div>"_AM;
	}
}
