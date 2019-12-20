#include "AlignRuleSet.hpp"

#include <sstream>

namespace Parser{
	std::string toStringNodeAlign(const NodeVariant& nod){
        const Align& align = std::get<Align>(nod);
        
        std::stringstream ss;
        ss << "Align:";
        switch(align.type){
        case Align::Type::Unknown:
            ss << "Unknown";
            break;
        case Align::Type::Center:
            ss << "Center";
            break;
        case Align::Type::Left:
            ss << "Left";
            break;
        case Align::Type::Right:
            ss << "Right";
            break;
        case Align::Type::Justify:
            ss << "Justify";
            break;
        }
        return ss.str();
	}
    
    void handleAlign(TreeContext& context, const Token& token){
        handleSectionStartEnd(token,
        [&](const SectionStart& section){
            Align::Type type;
            if(section.typeString == "="){
                type = Align::Type::Center;
            }
            else if(section.typeString == "<"){
                type = Align::Type::Left;
            }
            else if(section.typeString == ">"){
                type = Align::Type::Right;
            }
            else if(section.typeString == "=="){
                type = Align::Type::Justify;
            }
            makeDivPushable(context);
            pushStack(context, Node{Align{type}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Align)){//not actually correct, this should keep track of what kind of align it is talking about
                //but who's nesting different kind of aligns anyways? this work just fine, I'm sure...
                popSingle(context, Node::Type::Align);
            }
        });
    }
    
	void toHtmlNodeAlign(const HtmlContext& con, const Node& nod){
        const Align& node = std::get<Align>(nod.node);
        std::string textAlign;
        switch(node.type){
            case Align::Type::Center:
                textAlign = "center";
                break;
            case Align::Type::Left:
                textAlign = "left";
                break;
            case Align::Type::Right:
                textAlign = "right";
                break;
            case Align::Type::Justify:
                textAlign = "justify";
                break;
            default:
                throw std::runtime_error("Encountered invalid Align::Type");
        }
        con.out << "<div style='text-align:"_AM << textAlign << "'>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</div>"_AM;
	}
}
