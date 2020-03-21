#include "UserRuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeUser(const NodeVariant& nod){
        const User& user = std::get<User>(nod);
        nlohmann::json out;
        out["shownName"] = user.shownName;
        out["linkName"] = user.linkName;
        out["newWindow"] = user.newWindow;
        return out;
	}
	
    void handleUser(TreeContext& context, const Token& token){
        const Section& section = std::get<Section>(token.token);
        User user;
        user.shownName = section.mainParameter;
        user.linkName = normalizePageName(user.shownName);
        user.newWindow = (section.typeString == "*user");
        addAsText(context, Node{user});
    }
	
	void toHtmlNodeUser(const HtmlContext& con, const Node& nod){
        const User& user = std::get<User>(nod.node);
        con.out << "<a href='http://www.wikidot.com/user:info/"_AM << user.linkName << "'"_AM << (user.newWindow?" target='_blank'"_AM:""_AM) << ">"_AM << user.shownName << "</a>"_AM;
	}
}
