#include "CenterTextRuleSet.hpp"

namespace Parser{
    nlohmann::json printTokenCenterText(const TokenVariant& tok){
        return {};
    }
    
    nlohmann::json printNodeCenterText(const NodeVariant& nod){
        return {};
    }
    
	bool tryCenterTextRule(const TokenRuleContext& context){
        return context.wasNewLine && check(context.page, context.pagePos, "= ");
	}
	
	TokenRuleResult doCenterTextRule(const TokenRuleContext& context){
        TokenRuleResult result;
        result.newPos = context.pagePos + 2;
        result.newTokens.push_back(Token{CenterText{}, context.pagePos, result.newPos, context.page.substr(context.pagePos, result.newPos - context.pagePos)});
        return result;
	}
	
    void handleCenterText(TreeContext& context, const Token& token){
        makeDivPushable(context);
        pushStack(context, Node{CenterText{}});
    }
    
	void toHtmlNodeCenterText(const HtmlContext& con, const Node& nod){
        con.out << "<p style='text-align:center;'>"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</p>"_AM;
	}
}
