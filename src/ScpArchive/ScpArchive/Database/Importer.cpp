#include "Importer.hpp"

#include <iostream>

#include <boost/filesystem.hpp>

#include "Json.hpp"

namespace Importer{
	namespace{
		inline void setMapDetail(std::string raw, Database::ID id, std::map<std::string, Database::ID>& map){
			if(map.find(raw) != map.end()){
				throw std::runtime_error("Attempted to set ImportMap twice");
			}
			map[raw] = id;
		}
		
		inline Database::ID getMapDetail(std::string raw, std::map<std::string, Database::ID>& map){
			auto i = map.find(raw);
			if(i == map.end()){
				throw std::runtime_error("Attempted to get ImportMap but no mapping exists");
			}
			return i->second;
		}
	}

	void ImportMap::setPageMap(std::string raw, Database::ID id){
		setMapDetail(raw, id, pageMap);
	}
	
	Database::ID ImportMap::getPageMap(std::string raw){
		return getMapDetail(raw, pageMap);
	}
	
	void ImportMap::setFileMap(std::string raw, Database::ID id){
		setMapDetail(raw, id, fileMap);
	}
	
	Database::ID ImportMap::getFileMap(std::string raw){
		return getMapDetail(raw, fileMap);
	}
	
	void ImportMap::ImportMap::setThreadMap(std::string raw, Database::ID id){
        setMapDetail(raw, id, threadMap);
	}
	
    Database::ID ImportMap::ImportMap::getThreadMap(std::string raw){
        return getMapDetail(raw, threadMap);
    }
    
    void ImportMap::setCategoryMap(std::string raw, Database::ID id){
        setMapDetail(raw, id, categoryMap);
    }
    
    Database::ID ImportMap::getCategoryMap(std::string raw){
        return getMapDetail(raw, categoryMap);
    }
	
	namespace{
		uint64_t getTimeStamp(nlohmann::json time){
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
	
	void importThread(Database* database, ImportMap& map, nlohmann::json threadData){
		
	}
	
	void importBasicPageDataFromFolder(Database* database, ImportMap& map, std::string pagesDirectory){
		for(boost::filesystem::directory_iterator i(pagesDirectory); i != boost::filesystem::directory_iterator(); i++){
			if(boost::filesystem::is_directory(i->path())){
				std::cout << "Importing " << i->path().string() << "\n";
				importBasicPageData(database, map, Json::loadJsonFromFile(i->path().string() + "/data.json"));
			}
		}
	}
	
	void importBasicPageData(Database* database, ImportMap& map, nlohmann::json pageData){
		Database::ID pageId = *database->createPage(pageData["name"].get<std::string>());
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
			Database::ID fileId = *database->createPageFile(pageId, pageFile);
			map.setFileMap(file["id"].get<std::string>(), fileId);
		}
	}
	
	void linkPageParent(Database* database, ImportMap& map, nlohmann::json pageData){
		if(pageData["parent"].get<std::string>() != ""){
			database->setPageParent(map.getPageMap(pageData["id"].get<std::string>()), map.getPageMap(pageData["parent"].get<std::string>()));
		}
	}
	
	void linkPageDiscussionThread(Database* database, ImportMap& map, nlohmann::json pageData){
		
	}
	
	void uploadPageFiles(Database* database, ImportMap& map, nlohmann::json pageData){
		
	}
}
