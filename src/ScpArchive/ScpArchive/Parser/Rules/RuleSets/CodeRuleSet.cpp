#include "CodeRuleSet.hpp"

namespace Parser{
	std::string toStringNodeCode(const NodeVariant& nod){
        const Code& code = std::get<Code>(nod);
        return "Code:\"" + code.contents + "\"";
	}
    
    void handleCode(TreeContext& context, const Token& token){
        const SectionComplete& section = std::get<SectionComplete>(token.token);
        
        Code code;
        if(section.parameters.find("type") != section.parameters.end()){
            code.type = section.parameters.find("type")->second;
            std::transform(code.type.begin(), code.type.end(), code.type.begin(), ::tolower);
        }
        code.contents = section.contents;
        addAsDiv(context, Node{code});
        context.codeData.push_back(code);
    }
    
	void toHtmlNodeCode(const HtmlContext& con, const Node& nod){
        const Code& code = std::get<Code>(nod.node);
        
        con.out << "<div class='CodeBlock'>"_AM;
        for(const char c : code.contents){
            switch(c){
                default:
                    con.out << c;
                    break;
                case '\n':
                    con.out << "<br />"_AM;
                    break;
                case ' ':
                    con.out << "&ensp;"_AM;
                    break;
            }
        }
        con.out << "</div>"_AM;
	}
    
}
