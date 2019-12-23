#include "TabViewRuleSet.hpp"

namespace Parser{
    std::string toStringNodeTabView(const NodeVariant& nod){
        return "TabView";
    }
    
    std::string toStringNodeTab(const NodeVariant& nod){
        return "Tab:" + std::get<Tab>(nod).title;
    }
	
    void handleTabView(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeDivPushable(context);
            pushStack(context, Node{TabView{}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::TabView)){
                popSingle(context, Node::Type::TabView);
            }
        });
    }
    
    void handleTab(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            if(isInside(context, Node::Type::TabView)){
                while(context.stack.back().getType() != Node::Type::TabView){
                    popStackWithCarry(context);
                }
                pushStack(context, Node{Tab{section.mainParameter}});
            }
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Tab)){
                popSingle(context, Node::Type::Tab);
            }
        });
    }
	
	void toHtmlNodeTabView(const HtmlContext& con, const Node& nod){
        const TabView& tabView = std::get<TabView>(nod.node);
        
        std::string formId = getUniqueHtmlId(con);
        con.out << "<div class='TabView'>"_AM
        << "<div class='TabViewButtons'>"_AM
        << "<form id='"_AM << formId << "'></form>"_AM;
        std::vector<std::string> radioIds;
        for(auto i = nod.branches.begin(); i != nod.branches.end(); i++){
            const Tab& tab = std::get<Tab>(i->node);
            std::string radioId = getUniqueHtmlId(con);
            con.out << "<label class='TabViewButton' for='"_AM << radioId << "'>"_AM << tab.title << "</label>"_AM;
            radioIds.push_back(radioId);
        }
        con.out << "</div>"_AM;
        {
            std::size_t num = 0;
            for(auto i = nod.branches.begin(); i != nod.branches.end(); i++, num++){
                const Tab& tab = std::get<Tab>(i->node);
                con.out << "<input type='radio' name='"_AM << formId << "' id='"_AM << radioIds[num] << "'"_AM << (num==0?" checked":"") << "></input>"_AM;
                convertNodeToHtml(con, *i);
            }
        }
        con.out << "</div>"_AM;
	}
	
	void toHtmlNodeTab(const HtmlContext& con, const Node& nod){
        const Tab& tab = std::get<Tab>(nod.node);
        con.out << "<div class='TabSection'>"_AM
        << "<div class='TabSectionTitle'>"_AM << tab.title << "</div>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</div>"_AM;
	}
}
