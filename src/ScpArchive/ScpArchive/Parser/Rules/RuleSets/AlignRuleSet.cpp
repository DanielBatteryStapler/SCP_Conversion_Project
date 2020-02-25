#include "AlignRuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeAlign(const NodeVariant& nod){
        const Align& align = std::get<Align>(nod);
        switch(align.type){
			default:
				return "Unknown";
			case Align::Type::Center:
				return "Center";
			case Align::Type::Left:
				return "Left";
			case Align::Type::Right:
				return "Right";
			case Align::Type::Justify:
				return "Justify";
        }
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
