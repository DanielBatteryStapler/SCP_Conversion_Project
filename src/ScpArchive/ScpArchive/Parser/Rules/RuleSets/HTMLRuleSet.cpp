#include "HTMLRuleSet.hpp"

namespace Parser{
    nlohmann::json printNodeHTML(const NodeVariant& nod){
        const HTML& html = std::get<HTML>(nod);
        return html.contents;
    }
	
    void handleHTML(TreeContext& context, const Token& token){
        const SectionComplete& section = std::get<SectionComplete>(token.token);
        HTML html;
        html.contents = section.contents;
        addAsDiv(context, Node{html});
    }
	
	void toHtmlNodeHTML(const HtmlContext& con, const Node& nod){
        const HTML& html = std::get<HTML>(nod.node);
        con.out << "<iframe style='width:100%;height:auto;' frameborder='0' src='data:text/html,"_AM << "<!DOCTYPE html><html><body>";
        
        for(char c : html.contents){
			switch(c){
				default:
					con.out << c;
					break;
				case '#':
					con.out << "%23";
					break;
			}
        }
        con.out << "</body></html>" << "'></iframe>"_AM;
	}
}
