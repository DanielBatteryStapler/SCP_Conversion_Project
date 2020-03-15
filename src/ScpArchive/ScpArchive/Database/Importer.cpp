#include "Importer.hpp"

#include <iostream>

#include <boost/filesystem.hpp>

#include "Json.hpp"

namespace Importer{
	ImportMap::ImportMap(Database* database):
		db(database){
		
	}
	
	namespace{
		inline void setMapDetail(Database* db, Database::MapCategory category, std::string raw, Database::ID id){
			db->setIdMap(category, raw, id);
		}
		
		inline Database::ID getMapDetail(Database* db, Database::MapCategory category, std::string raw){
			std::optional<Database::ID> id = db->getIdMap(category, raw);
			if(!id){
				throw std::runtime_error("Attempted to get ImportMap but no mapping exists");
			}
			return *id;
		}
		
		inline std::string getMapRawDetail(Database* db, Database::MapCategory category, Database::ID id){
			std::optional<std::string> raw = db->getIdMapRaw(category, id);
			if(!raw){
				throw std::runtime_error("Attempted to get ImportMap raw but no mapping exists");
			}
			return *raw;
		}
		
		inline bool mapExistsDetail(Database* db, Database::MapCategory category, std::string raw){
			std::optional<Database::ID> id = db->getIdMap(category, raw);
			if(id){
                return true;
			}
			else{
                return false;
			}
		}
	}
	
	void ImportMap::setPageMap(std::string raw, Database::ID id){
		setMapDetail(db, Database::MapCategory::Page, raw, id);
	}
	
	Database::ID ImportMap::getPageMap(std::string raw){
		return getMapDetail(db, Database::MapCategory::Page, raw);
	}
	
	std::string ImportMap::getPageMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapCategory::Page, id);
	}
	
	bool ImportMap::pageMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapCategory::Page, raw);
	}
	
	void ImportMap::setFileMap(std::string raw, Database::ID id){
		setMapDetail(db, Database::MapCategory::File, raw, id);
	}
	
	Database::ID ImportMap::getFileMap(std::string raw){
		return getMapDetail(db, Database::MapCategory::File, raw);
	}
	
	std::string ImportMap::getFileMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapCategory::File, id);
	}
	
	bool ImportMap::fileMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapCategory::File, raw);
	}
	
	void ImportMap::setAuthorMap(std::string raw, Database::ID id){
        setMapDetail(db, Database::MapCategory::Author, raw, id);
    }
    
    Database::ID ImportMap::getAuthorMap(std::string raw){
        return getMapDetail(db, Database::MapCategory::Author, raw);
    }
    
    std::string ImportMap::getAuthorMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapCategory::Author, id);
	}
	
	bool ImportMap::authorMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapCategory::Author, raw);
	}
	
	namespace{
		int64_t getTimeStamp(nlohmann::json time){
			return std::stoll(time.get<std::string>());
		}
	}
	
	void importAuthors(Database* database, ImportMap& map, const nlohmann::json& authorsData){
        std::cout << "Importing Authors...\n";
        for(const nlohmann::json& author : authorsData){
            importAuthor(database, map, author);
        }
    }
	
	void importAuthor(Database* database, ImportMap& map, const nlohmann::json& authorData){
		Database::Author author;
		author.type = Database::Author::Type::User;
		author.name = authorData["name"].get<std::string>();
		
		std::string sourceId = authorData["id"].get<std::string>();
		if(map.authorMapExists(sourceId)){
            Database::ID id = map.getAuthorMap(sourceId);
            database->resetAuthor(id, author);
		}
		else{
            Database::ID id = database->createAuthor(author);
            map.setAuthorMap(sourceId, id);
		}
	}
	
	void importForumGroups(Database* database, ImportMap& map, const nlohmann::json& forumGroups){
		std::cout << "Importing Forum Group Data...\n";
		for(nlohmann::json jGroup : forumGroups){
            Database::ForumGroup group;
            group.title = jGroup["title"].get<std::string>();
            group.description = jGroup["description"].get<std::string>();
            
            Database::ID groupId = database->createForumGroup(group);
            for(nlohmann::json jCategory : jGroup["categories"]){
                Database::ForumCategory category;
                category.sourceId = jCategory["id"].get<std::string>();
                category.title = jCategory["title"].get<std::string>();
                category.description = jCategory["description"].get<std::string>();
                
                database->createForumCategory(groupId, category);
            }
		}
	}
	
	namespace{
		inline bool threadExists(const nlohmann::json& threadData){
			return !(threadData.find("nonExistent") != threadData.end() && threadData["nonExistent"].get<bool>());
		}
	}
	
	void importThreadsFromFolder(Database* database, ImportMap& map, std::string threadsDirectory, std::vector<std::string> threads){
		std::cout << "Importing Threads...\n";
		for(std::string thread : threads){
			std::string threadFolder = threadsDirectory + thread + "/";
			std::cout << "\tImporting Thread " << thread << "\n";
			nlohmann::json threadData = Json::loadJsonFromFile(threadFolder + "data.json");
			if(threadExists(threadData)){
				importThread(database, map, threadData);
			}
		}
	}
	
	void importThread(Database* database, ImportMap& map, const nlohmann::json& threadData){
		Database::ForumThread thread;
		thread.sourceId = threadData["id"].get<std::string>();
		thread.parent = *database->getForumCategoryId(threadData["categoryId"].get<std::string>());
		thread.title = threadData["title"].get<std::string>();
		thread.description = threadData["description"].get<std::string>();
		thread.timeStamp = getTimeStamp(threadData["timeStamp"]);
		if(threadData["authorId"].get<std::string>() != "deleted"){
            thread.authorId = map.getAuthorMap(threadData["authorId"].get<std::string>());
		}
		
		Database::ID threadId;
		if(database->getForumThreadId(threadData["id"].get<std::string>())){
			threadId = *database->getForumThreadId(threadData["id"].get<std::string>());
			database->resetForumThread(threadId, thread);
		}
		else{
			threadId = database->createForumThread(thread);
		}
		
		importPosts(database, map, threadData["posts"], threadId, {});
	}
	
	void importPosts(Database* database, ImportMap& map, const nlohmann::json& posts, Database::ID parentThread, std::optional<Database::ID> parentPost){
		for(const nlohmann::json& post : posts){
			Database::ForumPost newPost;
			newPost.parentThread = parentThread;
			newPost.parentPost = parentPost;
			newPost.title = post["title"].get<std::string>();
			newPost.content = post["content"].get<std::string>();
			newPost.timeStamp = getTimeStamp(post["timeStamp"]);
			if(post["authorId"].get<std::string>() != "deleted"){
                newPost.authorId = map.getAuthorMap(post["authorId"].get<std::string>());
            }   
			Database::ID postId = database->createForumPost(newPost);
			importPosts(database, map, post["posts"], parentThread, postId);
		}
	}
	
	namespace{
		inline bool pageExists(const nlohmann::json& pageData){
			return !(pageData.find("nonExistent") != pageData.end() && pageData["nonExistent"].get<bool>());
		}
	}
	
	void importPagesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages){
		std::cout << "Importing Pages...\n";
		for(std::string page : pages){
			std::string pageFolder = pagesDirectory + page + "/";
			std::cout << "\tImporting Page " << page << "\n";
			nlohmann::json pageData = Json::loadJsonFromFile(pageFolder + "data.json");
			if(pageExists(pageData)){
				importPage(database, map, pageData);
			}
		}
	}
	
	void importPage(Database* database, ImportMap& map, const nlohmann::json& pageData){
		Database::ID pageId = -1;
		{
			std::optional<Database::ID> existing = database->getPageId(pageData["name"].get<std::string>());
			if(existing){
				pageId = existing.value();
				database->resetPage(pageId, pageData["name"].get<std::string>());
			}
			else{
				pageId = database->createPage(pageData["name"].get<std::string>()).value();
			}
		}
		map.setPageMap(pageData["id"].get<std::string>(), pageId);
		{
			std::vector<std::string> tags = pageData["tags"];
			database->setPageTags(pageId, tags);
		}
		
		if(pageData["parent"].get<std::string>() != ""){
			database->setPageParent(pageId, pageData["parent"].get<std::string>());
		}
		if(pageData["discussionId"].get<std::string>() != ""){
			database->setPageDiscussion(pageId, pageData["discussionId"].get<std::string>());
		}
		
		for(nlohmann::json rev : pageData["revisions"]){
			Database::PageRevision revision;
			revision.title = rev["title"].get<std::string>();
			revision.timeStamp = getTimeStamp(rev["timeStamp"]);
			revision.changeMessage = rev["changeMessage"].get<std::string>();
			revision.changeType = rev["changeType"].get<std::string>();
			revision.sourceCode = rev["sourceCode"].get<std::string>();
			if(rev["authorId"].get<std::string>() != "deleted"){
                revision.authorId = map.getAuthorMap(rev["authorId"].get<std::string>());
			}
			database->createPageRevision(pageId, revision);
		}
		for(nlohmann::json file : pageData["files"]){
			Database::PageFile pageFile;
			pageFile.name = file["name"].get<std::string>();
			pageFile.description = file["description"].get<std::string>();
			pageFile.timeStamp = getTimeStamp(file["timeStamp"]);
			Database::ID fileId = database->createPageFile(pageId, pageFile).value();
			map.setFileMap(file["id"].get<std::string>(), fileId);
		}
	}
	
	void uploadPageFilesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages){
		for(std::string page : pages){
			std::string pageFolder = pagesDirectory + page + "/";
			std::cout << "Uploading files for " << pageFolder << "\n";
			nlohmann::json pageJson = Json::loadJsonFromFile(pageFolder + "data.json");
			if(pageExists(pageJson)){
				uploadPageFiles(database, map, pageJson, pageFolder);
			}
		}
	}
	
	void uploadPageFiles(Database* database, ImportMap& map, const nlohmann::json& pageData, std::string pageDirectory){
		for(nlohmann::json file : pageData["files"]){
			Database::ID fileId = map.getFileMap(file["id"].get<std::string>());
			std::string filePath = pageDirectory + "files/" + file["id"].get<std::string>();
			std::cout << "    Uploading File \"" << filePath << "\"...\n";
			std::ifstream fileStream(filePath);
			database->uploadPageFile(fileId, fileStream);
		}
	}
}
