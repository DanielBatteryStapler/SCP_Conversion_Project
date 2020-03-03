#include "ForumRuleSet.hpp"

#include "../../../Database/Database.hpp"
#include <cmath>

namespace Parser{
	nlohmann::json printNodeForumStart(const NodeVariant& nod){
		const ForumStart& forumStart = std::get<ForumStart>(nod);
		nlohmann::json out;
		out["groups"] = nlohmann::json::array();
		for(const ForumStart::Group& i : forumStart.groups){
			nlohmann::json groupJ;
			groupJ["title"] = i.title;
			groupJ["description"] = i.description;
			groupJ["categories"] = nlohmann::json::array();
			for(const ForumStart::Category& y : i.categories){
				nlohmann::json categoryJ;
				categoryJ["id"] = y.id;
				categoryJ["title"] = y.title;
				categoryJ["description"] = y.description;
				groupJ["categories"].push_back(categoryJ);
			}
			out["groups"].push_back(groupJ);
		}
		return out;
	}

	nlohmann::json printNodeForumCategory(const NodeVariant& nod){
		const ForumCategory& forumCategory = std::get<ForumCategory>(nod);
		nlohmann::json out;
		out["title"] = forumCategory.title;
		out["description"] = forumCategory.description;
		out["currentPage"] = forumCategory.currentPage;
		out["totalPages"] = forumCategory.totalPages;
		out["threads"] = nlohmann::json::array();
		for(const ForumCategory::Thread& i : forumCategory.threads){
			nlohmann::json threadJ;
			threadJ["id"] = i.id;
			threadJ["title"] = i.title;
			threadJ["description"] = i.description;
			threadJ["timeStamp"] = i.timeStamp;
			out["threads"].push_back(threadJ);
		}
		return out;
	}

	nlohmann::json printNodeForumThread(const NodeVariant& nod){
		const ForumThread& forumThread = std::get<ForumThread>(nod);
		nlohmann::json out;
		out["categoryId"] = forumThread.categoryId;
		out["title"] = forumThread.title;
		out["description"] = forumThread.description;
		out["timeStamp"] = forumThread.timeStamp;
		return out;
	}

	nlohmann::json printNodeForumPost(const NodeVariant& nod){
		const ForumPost& forumPost = std::get<ForumPost>(nod);
		nlohmann::json out;
		out["title"] = forumPost.title;
		out["content"] = forumPost.content;
		out["timeStamp"] = forumPost.timeStamp;
		return out;
	}

	void handleForumStart(TreeContext& context, const Token& token){
		Database* db = context.parameters.database;
		if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Forum failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
		ForumStart node;
		std::vector<Database::ID> groups = db->getForumGroups();
		for(Database::ID groupId : groups){
			Database::ForumGroup groupData = db->getForumGroup(groupId);
			ForumStart::Group group;
			group.title = groupData.title;
			group.description = groupData.description;
			std::vector<Database::ID> categories = db->getForumCategories(groupId);
			for(Database::ID categoryId : categories){
				Database::ForumCategory categoryData = db->getForumCategory(categoryId);
				ForumStart::Category category;
				category.title = categoryData.title;
				category.description = categoryData.description;
				group.categories.push_back(category);
			}
			node.groups.push_back(group);
		}
		addAsDiv(context, Node{node});
	}

	void handleForumCategory(TreeContext& context, const Token& token){
		Database* db = context.parameters.database;
		if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Forum failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
        Database::ID categoryId;
        {
			std::string categoryIdRaw;
			if(context.parameters.urlParameters.find("forum") != context.parameters.urlParameters.end()){
				categoryIdRaw = context.parameters.urlParameters["forum"];
				if(categoryIdRaw.size() > 2){
					categoryIdRaw.erase(0, 2);
					if(db->getForumCategoryId(categoryIdRaw)){
						categoryId = db->getForumCategoryId(categoryIdRaw).value();
					}
					else{
						categoryIdRaw = "";
					}
				}
				else{
					categoryIdRaw = "";
				}
			}
			if(categoryIdRaw == ""){
				makeDivPushable(context);
				addAsText(context, Node{PlainText{"Forum failed: invalid category id"}});
				makeDivPushable(context);
				return;
			}
        }
        int page = 0;
        if(context.parameters.urlParameters.find("p") != context.parameters.urlParameters.end()){
			try{
				page = std::stoll(context.parameters.urlParameters["p"]);
			}
			catch(std::exception& e){
				//if there's an error, just use the default value
			}
        }
        Database::ForumCategory category = db->getForumCategory(categoryId);
		ForumCategory node;
		node.title = category.title;
		node.description = category.description;
		node.currentPage = page;
		constexpr std::int64_t threadsPerPage = 25;
		node.totalPages = std::ceil(static_cast<long double>(db->getNumberOfForumThreads(categoryId)) / threadsPerPage);
		{
			std::vector<Database::ID> threads = db->getForumThreads(categoryId, threadsPerPage, threadsPerPage * page);
			for(Database::ID threadId : threads){
				Database::ForumThread threadData = db->getForumThread(threadId);
				ForumCategory::Thread thread;
				thread.id = threadData.sourceId;
				thread.timeStamp = threadData.timeStamp;
				thread.title = threadData.title;
				thread.description = threadData.description;
				node.threads.push_back(thread);
			}
		}
		addAsDiv(context, Node{node});
	}

	void handleForumThread(TreeContext& context, const Token& token){
		Database* db = context.parameters.database;
		if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Forum failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
        Database::ID threadId;
        {
			std::string threadIdRaw;
			if(context.parameters.urlParameters.find("forum") != context.parameters.urlParameters.end()){
				threadIdRaw = context.parameters.urlParameters["forum"];
				if(threadIdRaw.size() > 2){
					threadIdRaw.erase(0, 2);
					if(db->getForumThreadId(threadIdRaw)){
						threadId = db->getForumThreadId(threadIdRaw).value();
					}
					else{
						threadIdRaw = "";
					}
				}
				else{
					threadIdRaw = "";
				}
			}
			if(threadIdRaw == ""){
				makeDivPushable(context);
				addAsText(context, Node{PlainText{"Forum failed: invalid thread id"}});
				makeDivPushable(context);
				return;
			}
        }
        constexpr std::int64_t postsPerPage = 25;
        int page = 0;
        if(context.parameters.urlParameters.find("p") != context.parameters.urlParameters.end()){
			try{
				page = std::stoll(context.parameters.urlParameters["p"]);
			}
			catch(std::exception& e){
				//if there's an error, just use the default value
			}
        }
        ForumThread thread;
        {
			Database::ForumThread threadData = db->getForumThread(threadId);
			thread.categoryId = db->getForumCategory(threadData.parent).sourceId;
			thread.title = threadData.title;
			thread.description = threadData.description;
			thread.timeStamp = threadData.timeStamp;
			thread.currentPage = page;
			thread.totalPages = std::ceil(static_cast<long double>(db->getNumberOfForumReplies(threadId)) / postsPerPage);
        }
        Node node{thread};
        //now we just gotta make a tree of all of the replies
        const std::function<void(Node&, Database::ID, std::optional<Database::ID>)> getReplies
		= [db, &getReplies, &page, postsPerPage](Node& node, Database::ID threadId, std::optional<Database::ID> postId)->void{
			std::vector<Database::ID> replies;
			if(postId){
                replies = db->getForumReplies(threadId, *postId, 999, 0);
                //for now have a ridiculous limit for nested post counts
			}
			else{
                replies = db->getForumReplies(threadId, {}, postsPerPage, postsPerPage * page);
			}
			for(Database::ID reply : replies){
				Database::ForumPost postData = db->getForumPost(reply);
				ForumPost post;
				post.title = postData.title;
				post.content = postData.content;
				post.timeStamp = postData.timeStamp;
				Node postNode{post};
				getReplies(postNode, threadId, reply);
				node.branches.push_back(postNode);
			}
        };
        getReplies(node, threadId, {});
		addAsDiv(context, node);
	}

	void toHtmlNodeForumStart(const HtmlContext& con, const Node& nod){
		const ForumStart& forum = std::get<ForumStart>(nod.node);
		con.out << "<table><tbody>"_AM;
		for(const ForumStart::Group& group : forum.groups){
			con.out << "<tr><th>"_AM << group.title << "</th><th>"_AM << group.description << "</th></tr>"_AM;
			for(const ForumStart::Category& category : group.categories){
				con.out << "<tr><td><a href='/forum/c-"_AM << category.id << "'>"_AM << category.title << "</a></td><td>"_AM
				<< category.description << "</td></tr>"_AM;
			}
		}
		con.out << "</tbody></table>"_AM;
	}

	void toHtmlNodeForumCategory(const HtmlContext& con, const Node& nod){
		const ForumCategory& forum = std::get<ForumCategory>(nod.node);
		con.out << "<h1>"_AM << forum.title << "</h1><p>"_AM << forum.description << "</p><table><tbody>"_AM;
		for(const ForumCategory::Thread& thread : forum.threads){
			con.out << "<tr><td><a href='/forum/t-"_AM << thread.id << "'>"_AM << thread.title << "</td></tr>"_AM;
		}
		con.out << "</tbody></table>"_AM;
	}

	void toHtmlNodeForumThread(const HtmlContext& con, const Node& nod){
		const ForumThread& forum = std::get<ForumThread>(nod.node);
		con.out << "<div style='border-color:black;border-style:solid;border-width:1px;'><h1>"_AM
		<< forum.title << "</h1>"_AM << forum.description << "</div>"_AM;
		delegateNodeBranches(con, nod);
	}

	void toHtmlNodeForumPost(const HtmlContext& con, const Node& nod){
		const ForumPost& forum = std::get<ForumPost>(nod.node);
		con.out << "<div style='margin:10px;border-color:black;border-style:solid;border-width:1px;'><h3>"_AM
		<< forum.title << "</h3>"_AM << allowMarkup(forum.content);///TODO: fix this! this opens up this website to a bunch of security issues
		delegateNodeBranches(con, nod);
		con.out << "</div>"_AM;
	}
}
