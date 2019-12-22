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
        con.out << "<iframe src='"_AM << iframe.source << "' "_AM;
        for(auto i = iframe.parameters.begin(); i != iframe.parameters.end(); i++){
            con.out << i->first << "='"_AM << i->second << "' "_AM;
        }
        con.out << "></iframe>"_AM;
	}
}
