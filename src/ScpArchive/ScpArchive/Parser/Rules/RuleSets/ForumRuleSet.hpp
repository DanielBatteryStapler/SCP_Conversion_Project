#ifndef FORUMRULESET_HPP
#define FORUMRULESET_HPP

#include "../RuleSet.hpp"

namespace Parser{
	nlohmann::json printNodeForumStart(const NodeVariant& nod);
	nlohmann::json printNodeForumCategory(const NodeVariant& nod);
	nlohmann::json printNodeForumThread(const NodeVariant& nod);
	nlohmann::json printNodeForumPost(const NodeVariant& nod);
	
    void handleForumStart(TreeContext& context, const Token& token);
    void handleForumCategory(TreeContext& context, const Token& token);
    void handleForumThread(TreeContext& context, const Token& token);
    
	void toHtmlNodeForumStart(const HtmlContext& con, const Node& nod);
	void toHtmlNodeForumCategory(const HtmlContext& con, const Node& nod);
	void toHtmlNodeForumThread(const HtmlContext& con, const Node& nod);
	void toHtmlNodeForumPost(const HtmlContext& con, const Node& nod);
	
	const inline auto forumRuleSet = RuleSet{"Forum", {
		NodePrintRule{Node::Type::ForumStart, printNodeForumStart},
		NodePrintRule{Node::Type::ForumCategory, printNodeForumCategory},
		NodePrintRule{Node::Type::ForumThread, printNodeForumThread},
		NodePrintRule{Node::Type::ForumPost, printNodeForumPost},
		
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::ForumStart, {"forumstart"},
            ContentType::None, ParameterType::None, false},
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::ForumCategory, {"forumcategory"},
            ContentType::None, ParameterType::None, false},
		SectionRule{SectionType::Module, {"module"}, SubnameType::Module, ModuleType::ForumThread, {"forumthread"},
            ContentType::None, ParameterType::None, false},
		
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::ForumStart}, handleForumStart},
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::ForumCategory}, handleForumCategory},
        TreeRule{{Token::Type::Section, SectionType::Module, ModuleType::ForumThread}, handleForumThread},
        
        HtmlRule{Node::Type::ForumStart, toHtmlNodeForumStart},
        HtmlRule{Node::Type::ForumCategory, toHtmlNodeForumCategory},
        HtmlRule{Node::Type::ForumThread, toHtmlNodeForumThread},
        HtmlRule{Node::Type::ForumPost, toHtmlNodeForumPost}
	}};
}

#endif // FORUMRULESET_HPP
