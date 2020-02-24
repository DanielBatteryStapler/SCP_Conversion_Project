#include "CollapsibleRuleSet.hpp"

#include <sstream>

namespace Parser{
    nlohmann::json printNodeCollapsible(const NodeVariant& nod){
        const Collapsible& collapsible = std::get<Collapsible>(nod);
        nlohmann::json out;
        out["openedText"] = collapsible.openedText;
        out["closedText"] = collapsible.closedText;
        out["defaultShow"] = collapsible.defaultShow;
        return out;
    }
    
    void handleCollapsible(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            Collapsible collapsible;
            if(section.parameters.find("show") != section.parameters.end()){
                collapsible.closedText = section.parameters.find("show")->second;
            }
            else{
                collapsible.closedText = "+ show block";
            }
            
            if(section.parameters.find("hide") != section.parameters.end()){
                collapsible.openedText = section.parameters.find("hide")->second;
            }
            else{
                collapsible.openedText = "- hide block";
            }
            
            if(section.parameters.find("folded") != section.parameters.end()){
                if(section.parameters.find("folded")->second == "no"){
                    collapsible.defaultShow = true;
                }
                else{
                    collapsible.defaultShow = false;
                }
            }
            else{
                collapsible.defaultShow = false;
            }
            
            makeDivPushable(context);
            pushStack(context, Node{collapsible});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Collapsible)){
                popSingle(context, Node::Type::Collapsible);
            }
        });
    }
    
	void toHtmlNodeCollapsible(const HtmlContext& con, const Node& nod){
        const Collapsible& collapsible = std::get<Collapsible>(nod.node);
        
        std::string uniqueId = getUniqueHtmlId(con);
        
		con.out << "<div class='CollapsibleContainer'>"_AM
		<< "<input type='checkbox' id='"_AM << uniqueId << "' "_AM << (collapsible.defaultShow?"checked"_AM:""_AM) << ">"_AM
		<< "<label class='CollapsibleHiddenText' for='"_AM << uniqueId << "'>"_AM
		<< collapsible.closedText
		<< "</label>"_AM
		<< "<label class='CollapsibleShownText' for='"_AM << uniqueId << "'>"_AM
		<< collapsible.openedText
		<< "</label>"_AM
		<< "<div class='CollapsibleContent'>"_AM;
		
        delegateNodeBranches(con, nod);
        
		con.out << "</div></div>"_AM;
	}
}
