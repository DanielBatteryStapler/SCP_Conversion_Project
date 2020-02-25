#include "Importer.hpp"

#include <iostream>

#include <boost/filesystem.hpp>

#include "Json.hpp"

namespace Importer{
	ImportMap::ImportMap(Database* database):
		db(database){
		
	}
	
	namespace{
		inline void setMapDetail(Database* db, Database::MapType type, std::string raw, Database::ID id){
			db->setIdMap(static_cast<short>(type), raw, id);
		}
		
		inline Database::ID getMapDetail(Database* db, Database::MapType type, std::string raw){
			std::optional<Database::ID> id = db->getIdMap(static_cast<short>(type), raw);
			if(!id){
				throw std::runtime_error("Attempted to get ImportMap but no mapping exists");
			}
			return *id;
		}
		
		inline std::string getMapRawDetail(Database* db, Database::MapType type, Database::ID id){
			std::optional<std::string> raw = db->getIdMapRaw(static_cast<short>(type), id);
			if(!raw){
				throw std::runtime_error("Attempted to get ImportMap raw but no mapping exists");
			}
			return *raw;
		}
		
		inline bool mapExistsDetail(Database* db, Database::MapType type, std::string raw){
			std::optional<Database::ID> id = db->getIdMap(static_cast<short>(type), raw);
			if(id){
                return true;
			}
			else{
                return false;
			}
		}
	}
	
	void ImportMap::setPageMap(std::string raw, Database::ID id){
		setMapDetail(db, Database::MapType::Page, raw, id);
	}
	
	Database::ID ImportMap::getPageMap(std::string raw){
		return getMapDetail(db, Database::MapType::Page, raw);
	}
	
	std::string ImportMap::getPageMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapType::Page, id);
	}
	
	bool ImportMap::pageMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapType::Page, raw);
	}
	
	void ImportMap::setFileMap(std::string raw, Database::ID id){
		setMapDetail(db, Database::MapType::File, raw, id);
	}
	
	Database::ID ImportMap::getFileMap(std::string raw){
		return getMapDetail(db, Database::MapType::File, raw);
	}
	
	std::string ImportMap::getFileMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapType::File, id);
	}
	
	bool ImportMap::fileMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapType::File, raw);
	}
	
	void ImportMap::ImportMap::setThreadMap(std::string raw, Database::ID id){
        setMapDetail(db, Database::MapType::Thread, raw, id);
	}
	
    Database::ID ImportMap::ImportMap::getThreadMap(std::string raw){
        return getMapDetail(db, Database::MapType::Thread, raw);
    }
    
    std::string ImportMap::getThreadMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapType::Thread, id);
	}
    
    bool ImportMap::threadMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapType::Thread, raw);
	}
    
    void ImportMap::setCategoryMap(std::string raw, Database::ID id){
        setMapDetail(db, Database::MapType::Category, raw, id);
    }
    
    Database::ID ImportMap::getCategoryMap(std::string raw){
        return getMapDetail(db, Database::MapType::Category, raw);
    }
    
    std::string ImportMap::getCategoryMapRaw(Database::ID id){
		return getMapRawDetail(db, Database::MapType::Category, id);
	}
	
	bool ImportMap::categoryMapExists(std::string raw){
		return mapExistsDetail(db, Database::MapType::Category, raw);
	}
	
	namespace{
		int64_t getTimeStamp(nlohmann::json time){
			return std::stoll(time.get<std::string>());
		}
	}
	
	void importForumGroups(Database* database, ImportMap& map, nlohmann::json forumGroups){
		for(nlohmann::json jGroup : forumGroups){
            Database::ForumGroup group;
            group.title = jGroup["title"].get<std::string>();
            group.description = jGroup["description"].get<std::string>();
            
            Database::ID groupId = database->createForumGroup(group);
            for(nlohmann::json jCategory : jGroup["categories"]){
                Database::ForumCategory category;
                category.title = jCategory["title"].get<std::string>();
                category.description = jCategory["description"].get<std::string>();
                
                Database::ID categoryId = database->createForumCategory(groupId, category);
                map.setCategoryMap(jCategory["id"].get<std::string>(), categoryId);
            }
		}
	}
	
	namespace{
		inline bool threadExists(const nlohmann::json& threadData){
			return !(threadData.find("nonExistent") != threadData.end() && threadData["nonExistent"].get<bool>());
		}
	}
	
	void importThreadsFromFolder(Database* database, ImportMap& map, std::string threadsDirectory, std::vector<std::string> threads){
		for(std::string thread : threads){
			std::string threadFolder = threadsDirectory + thread + "/";
			std::cout << "Importing thread " << thread << "\n";
			nlohmann::json threadData = Json::loadJsonFromFile(threadFolder + "data.json");
			if(threadExists(threadData)){
				importThread(database, map, threadData);
			}
		}
	}
	
	void importThread(Database* database, ImportMap& map, nlohmann::json threadData){
		Database::ForumThread thread;
		thread.parent = map.getCategoryMap(threadData["categoryId"].get<std::string>());
		thread.title = threadData["title"].get<std::string>();
		thread.description = threadData["description"].get<std::string>();
		thread.timeStamp = getTimeStamp(threadData["timeStamp"]);
		
		Database::ID threadId = -1;
		{
			if(map.threadMapExists(threadData["id"].get<std::string>())){
				threadId = map.getThreadMap(threadData["id"].get<std::string>());
				database->resetForumThread(threadId, thread);
			}
			else{
				threadId = database->createForumThread(thread);
			}
		}
		map.setThreadMap(threadData["id"].get<std::string>(), threadId);
		
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
			Database::ID postId = database->createForumPost(newPost);
			importPosts(database, map, post["posts"], parentThread, postId);
		}
	}
	
	namespace{
		inline bool pageExists(const nlohmann::json& pageData){
			return !(pageData.find("nonExistent") != pageData.end() && pageData["nonExistent"].get<bool>());
		}
	}
	
	void importBasicPageDataFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages){
		for(std::string page : pages){
			std::string pageFolder = pagesDirectory + page + "/";
			std::cout << "Importing page " << page << "\n";
			nlohmann::json pageData = Json::loadJsonFromFile(pageFolder + "data.json");
			if(pageExists(pageData)){
				importBasicPageData(database, map, pageData);
			}
		}
	}
	
	void importBasicPageData(Database* database, ImportMap& map, nlohmann::json pageData){
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
		
		for(nlohmann::json rev : pageData["revisions"]){
			Database::PageRevision revision;
			revision.title = rev["title"].get<std::string>();
			revision.timeStamp = getTimeStamp(rev["timeStamp"]);
			revision.changeMessage = rev["changeMessage"].get<std::string>();
			revision.changeType = rev["changeType"].get<std::string>();
			revision.sourceCode = rev["sourceCode"].get<std::string>();
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
	
	void performPageDataLinksFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages){
		for(std::string page : pages){
			std::string pageFolder = pagesDirectory + page + "/";
			std::cout << "Linking " << pageFolder << "\n";
			nlohmann::json pageJson = Json::loadJsonFromFile(pageFolder + "data.json");
			
			if(pageExists(pageJson)){
				linkPageParent(database, map, pageJson);
				linkPageDiscussionThread(database, map, pageJson);
			}
		}
	}
	
	void linkPageParent(Database* database, ImportMap& map, nlohmann::json pageData){
		if(pageData["parent"].get<std::string>() != ""){
			database->setPageParent(map.getPageMap(pageData["id"].get<std::string>()), pageData["parent"].get<std::string>());
		}
	}
	
	void linkPageDiscussionThread(Database* database, ImportMap& map, nlohmann::json pageData){
		if(pageData["discussionId"].get<std::string>() != ""){
			database->setPageDiscussion(map.getPageMap(pageData["id"].get<std::string>()), map.getThreadMap(pageData["discussionId"].get<std::string>()));
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
	
	void uploadPageFiles(Database* database, ImportMap& map, nlohmann::json pageData, std::string pageDirectory){
		for(nlohmann::json file : pageData["files"]){
			Database::ID fileId = map.getFileMap(file["id"].get<std::string>());
			std::string filePath = pageDirectory + "files/" + file["id"].get<std::string>();
			std::cout << "    Uploading File \"" << filePath << "\"...\n";
			std::ifstream fileStream(filePath);
			database->uploadPageFile(fileId, fileStream);
		}
	}
}
