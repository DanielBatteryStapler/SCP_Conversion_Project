#include "HTMLRuleSet.hpp"

namespace Parser{
    std::string toStringNodeHTML(const NodeVariant& nod){
        const HTML& html = std::get<HTML>(nod);
        return "HTML:\"" + html.contents + "\"";
    }
	
    void handleHTML(TreeContext& context, const Token& token){
        const SectionComplete& section = std::get<SectionComplete>(token.token);
        HTML html;
        html.contents = section.contents;
        addAsDiv(context, Node{html});
    }
	
	void toHtmlNodeHTML(const HtmlContext& con, const Node& nod){
        const HTML& html = std::get<HTML>(nod.node);
        con.out << "<iframe style='width:100%;height:auto;' frameborder='0' src='data:text/html,"_AM << "<!DOCTYPE html><html><body>" << html.contents << "</body></html>" << "'></iframe>"_AM;
        //con.out << allowMarkup(html.contents);
	}
}
