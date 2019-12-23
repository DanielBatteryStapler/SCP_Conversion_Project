#include "IFrameRuleSet.hpp"

#include <sstream>

namespace Parser{
    std::string toStringNodeIFrame(const NodeVariant& nod){
        const IFrame& iframe = std::get<IFrame>(nod);
        std::stringstream ss;
        ss << "IFrame:" << iframe.source << ", {";
        for(auto i = iframe.parameters.begin(); i != iframe.parameters.end(); i++){
            ss << i->first << ": " << i->second << ", ";
        }
        ss << "}";
        return ss.str();
    }
	
    void handleIFrame(TreeContext& context, const Token& token){
        const Section& section = std::get<Section>(token.token);
        IFrame iframe;
        iframe.source = section.mainParameter;
        iframe.parameters = section.parameters;
        addAsDiv(context, Node{iframe});
    }
	
	void toHtmlNodeIFrame(const HtmlContext& con, const Node& nod){
        const IFrame& iframe = std::get<IFrame>(nod.node);
        con.out << "<iframe src='"_AM << iframe.source << "' align "_AM;
        for(auto i = iframe.parameters.begin(); i != iframe.parameters.end(); i++){
            if(i->first == "id" && check(i->second, 0, "u-") == false){
                con.out << " "_AM << i->first << "='u-"_AM << i->second << "'"_AM;
            }
            else{
                con.out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
        }
        con.out << "class style></iframe>"_AM;
	}
}
