#include "ForumRuleSet.hpp"

#include "../../../Database/Database.hpp"
#include <cmath>

namespace Parser{
	namespace{
        ForumAuthor getForumAuthor(Database* db, std::optional<Database::ID> authorId){
            ForumAuthor out;
            if(authorId){
                Database::Author author = db->getAuthor(authorId.value());
                switch(author.type){
                    default:
                        throw std::runtime_error("Got Author from Database with invalid Author::Type");
                        break;
                    case Database::Author::Type::System:
                        out.type = ForumAuthor::Type::System;
                        break;
                    case Database::Author::Type::User:
                        out.type = ForumAuthor::Type::User;
                        break;
                }
                out.name = author.name;
            }
            else{
                out.type = ForumAuthor::Type::Deleted;
            }
            return out;
        }
        
        nlohmann::json printForumAuthor(const ForumAuthor& author){
            nlohmann::json out;
            switch(author.type){
                default:
                    throw std::runtime_error("Attempted to print ForumAuthor with invalid Type");
                    break;
                case ForumAuthor::Type::System:
                    out["type"] = "System";
                    break;
                case ForumAuthor::Type::User:
                    out["type"] = "User";
                    break;
                case ForumAuthor::Type::Deleted:
                    out["type"] = "Deleted";
                    break;
            }
            out["name"] = author.name;
            return out;
        }
        
        void toHtmlForumAuthor(const HtmlContext& con, const ForumAuthor& author){
            switch(author.type){
                default:
                    throw std::runtime_error("Attempted to print ForumAuthor with invalid Type");
                    break;
                case ForumAuthor::Type::System:
                    con.out << "[SYSTEM]";
                    break;
                case ForumAuthor::Type::User:
                    con.out << "<a href='http://www.wikidot.com/user:info/"_AM << normalizePageName(author.name) << "'>"_AM << author.name << "</a>"_AM;
                    break;
                case ForumAuthor::Type::Deleted:
                    con.out << "[DELETED]";
                    break;
            }
        }
	}
	
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
		out["id"] = forumCategory.id;
		out["title"] = forumCategory.title;
		out["description"] = forumCategory.description;
		out["currentPage"] = forumCategory.currentPage;
		out["totalPages"] = forumCategory.totalPages;
		out["threads"] = nlohmann::json::array();
		for(const ForumCategory::Thread& i : forumCategory.threads){
			nlohmann::json threadJ;
			threadJ["id"] = i.id;
			threadJ["title"] = i.title;
			threadJ["author"] = printForumAuthor(i.author);
			threadJ["description"] = i.description;
			threadJ["timeStamp"] = i.timeStamp;
			out["threads"].push_back(threadJ);
		}
		return out;
	}

	nlohmann::json printNodeForumThread(const NodeVariant& nod){
		const ForumThread& forumThread = std::get<ForumThread>(nod);
		nlohmann::json out;
		out["id"] = forumThread.id;
		out["categoryId"] = forumThread.categoryId;
		out["title"] = forumThread.title;
        out["author"] = printForumAuthor(forumThread.author);
		out["description"] = forumThread.description;
		out["timeStamp"] = forumThread.timeStamp;
		return out;
	}

	nlohmann::json printNodeForumPost(const NodeVariant& nod){
		const ForumPost& forumPost = std::get<ForumPost>(nod);
		nlohmann::json out;
		out["title"] = forumPost.title;
		out["author"] = printForumAuthor(forumPost.author);
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
				category.id = categoryData.sourceId;
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
        std::optional<Database::ID> categoryId;
		std::string categoryIdSource;
		if(context.parameters.urlParameters.find("forum") != context.parameters.urlParameters.end()){
			categoryIdSource = context.parameters.urlParameters["forum"];
			if(categoryIdSource.size() > 2){
				categoryIdSource.erase(0, 2);
				categoryId = db->getForumCategoryId(categoryIdSource);
			}
		}
		if(!categoryId){
			makeDivPushable(context);
			addAsText(context, Node{PlainText{"Forum failed: invalid category id"}});
			makeDivPushable(context);
			return;
		}
		
		constexpr std::int64_t threadsPerPage = 20;
        int page = 0;
        if(context.parameters.urlParameters.find("p") != context.parameters.urlParameters.end()){
			try{
				page = std::stoll(context.parameters.urlParameters["p"]);
			}
			catch(std::exception& e){
				//if there's an error, just use the default value
			}
        }
        
		ForumCategory node;
		{
			Database::ForumCategory category = db->getForumCategory(categoryId.value());
			node.id = categoryIdSource;
			node.title = category.title;
			node.description = category.description;
			node.currentPage = page;
			node.totalPages = std::ceil(static_cast<long double>(db->getNumberOfForumThreads(categoryId.value())) / threadsPerPage);
		}
		{
			std::vector<Database::ID> threads = db->getForumThreads(categoryId.value(), threadsPerPage, threadsPerPage * page);
			for(Database::ID threadId : threads){
				ForumCategory::Thread thread;
				{
                    Database::ForumThread threadData = db->getForumThread(threadId);
                    thread.id = threadData.sourceId;
                    thread.timeStamp = threadData.timeStamp;
                    thread.title = threadData.title;
                    thread.author = getForumAuthor(db, threadData.authorId);
                    thread.description = threadData.description;
				}
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
        std::optional<Database::ID> threadId;
		std::string threadIdSource;
		if(context.parameters.urlParameters.find("forum") != context.parameters.urlParameters.end()){
			threadIdSource = context.parameters.urlParameters["forum"];
			if(threadIdSource.size() > 2){
				threadIdSource.erase(0, 2);
				threadId = db->getForumThreadId(threadIdSource);
			}
		}
		if(!threadId){
			makeDivPushable(context);
			addAsText(context, Node{PlainText{"Forum failed: invalid thread id"}});
			makeDivPushable(context);
			return;
		}
        
        constexpr std::int64_t postsPerPage = 10;
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
			Database::ForumThread threadData = db->getForumThread(threadId.value());
			thread.id = threadIdSource;
			thread.categoryId = db->getForumCategory(threadData.parent).sourceId;
			thread.title = threadData.title;
            thread.author = getForumAuthor(db, threadData.authorId);
			thread.description = threadData.description;
			thread.timeStamp = threadData.timeStamp;
			thread.currentPage = page;
			thread.totalPages = std::ceil(static_cast<long double>(db->getNumberOfForumReplies(threadId.value())) / postsPerPage);
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
				ForumPost post;
				{
                    Database::ForumPost postData = db->getForumPost(reply);
                    post.title = postData.title;
                    post.author = getForumAuthor(db, postData.authorId);
                    post.content = postData.content;
                    post.timeStamp = postData.timeStamp;
				}
                Node postNode{post};
                getReplies(postNode, threadId, reply);
				node.branches.push_back(postNode);
			}
        };
        getReplies(node, threadId.value(), {});
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
		const auto makePageSelect = [&](){
			con.out << "<div class='pageSelect'>"_AM;
			const auto makePageButton = [&forum, &con](int page){
				std::string pageStr = std::to_string(page);
				con.out
				<< "<a href='/forum/c-"_AM << forum.id << "/p/"_AM << pageStr << "'>"_AM
				<< "<div class='pageButton'>"_AM << pageStr << "</div></a>"_AM;
			};
			
			if(forum.currentPage > 0){
				makePageButton(0);
			}
			if(forum.currentPage > 2){
				con.out << "...";
			}
			if(forum.currentPage > 1){
				makePageButton(forum.currentPage - 1);
			}
			con.out << "<div class='pageButton currentPageButton'>"_AM << std::to_string(forum.currentPage) << "</div>"_AM;
			if(forum.currentPage < forum.totalPages - 2){
				makePageButton(forum.currentPage + 1);
			}
			if(forum.currentPage < forum.totalPages - 3){
				con.out << "...";
			}
			if(forum.currentPage < forum.totalPages - 1){
				makePageButton(forum.totalPages - 1);
			}
			con.out << "</div>"_AM;
		};
		makePageSelect();
		for(const ForumCategory::Thread& thread : forum.threads){
			con.out << "<tr><td><a href='/forum/t-"_AM << thread.id << "'>"_AM << thread.title << "</td>"_AM
			<< "<td>"_AM;
			toHtmlForumAuthor(con, thread.author);
			con.out << "</td>"_AM
			<< "<td>"_AM << formatTimeStamp(thread.timeStamp) << "</td></tr>"_AM;
		}
		makePageSelect();
		con.out << "</tbody></table>"_AM;
	}

	void toHtmlNodeForumThread(const HtmlContext& con, const Node& nod){
		const ForumThread& forum = std::get<ForumThread>(nod.node);
		con.out
		<< "<div class='threadContainer'>"_AM
		<< "<div class='threadTitle'>"_AM << forum.title << "</div>"_AM
		<< "<div class='threadInfo'>by "_AM;
		toHtmlForumAuthor(con, forum.author);
		con.out << " at "
		<< formatTimeStamp(forum.timeStamp)
		<< "</div>"_AM
		<< "<div class='threadDescription'>"_AM << allowMarkup(forum.description) << "</div>"_AM///TODO: fix allowMarkup(...) security hole
		<< "</div>"_AM;
		const auto makePageSelect = [&](){
			con.out << "<div class='pageSelect'>"_AM;
			const auto makePageButton = [&forum, &con](int page){
				std::string pageStr = std::to_string(page);
				con.out
				<< "<a href='/forum/t-"_AM << forum.id << "/p/"_AM << pageStr << "'>"_AM
				<< "<div class='pageButton'>"_AM << pageStr << "</div></a>"_AM;
			};
			
			if(forum.currentPage > 0){
				makePageButton(0);
			}
			if(forum.currentPage > 2){
				con.out << "...";
			}
			if(forum.currentPage > 1){
				makePageButton(forum.currentPage - 1);
			}
			con.out << "<div class='pageButton currentPageButton'>"_AM << std::to_string(forum.currentPage) << "</div>"_AM;
			if(forum.currentPage < forum.totalPages - 2){
				makePageButton(forum.currentPage + 1);
			}
			if(forum.currentPage < forum.totalPages - 3){
				con.out << "...";
			}
			if(forum.currentPage < forum.totalPages - 1){
				makePageButton(forum.totalPages - 1);
			}
			con.out << "</div>"_AM;
		};
		makePageSelect();
		delegateNodeBranches(con, nod);
		makePageSelect();
	}

	void toHtmlNodeForumPost(const HtmlContext& con, const Node& nod){
		const ForumPost& forum = std::get<ForumPost>(nod.node);
		con.out << "<div class='postContainer'>"_AM
		<< "<div class='postHeader'>"_AM
		<< "<div class='postTitle'>"_AM << forum.title << "</div>"_AM
		<< "<div class='postInfo'>by "_AM;
		toHtmlForumAuthor(con, forum.author);
		con.out << " at "
		<< formatTimeStamp(forum.timeStamp)
		<< "</div>"_AM
		<< "</div>"_AM
		<< "<div class='postContent'>"_AM << allowMarkup(forum.content) << "</div>"_AM;///TODO: fix allowMarkup(...) security hole
		delegateNodeBranches(con, nod);
		con.out << "</div>"_AM;
	}
	
	
}




















