#include "Importer.hpp"

#include <iostream>

#include <boost/filesystem.hpp>

#include "Json.hpp"

namespace Importer{
	ImportMap::ImportMap(Database* database):
		db(database){
		
	}
	
	namespace{
		inline void setMapDetail(Database* db, ImportMap::MapType type, std::string raw, Database::ID id){
			db->setIdMap(static_cast<short>(type), raw, id);
		}
		
		inline Database::ID getMapDetail(Database* db, ImportMap::MapType type, std::string raw){
			std::optional<Database::ID> id = db->getIdMap(static_cast<short>(type), raw);
			if(!id){
				throw std::runtime_error("Attempted to get ImportMap but no mapping exists");
			}
			return *id;
		}
	}
	
	void ImportMap::setPageMap(std::string raw, Database::ID id){
		setMapDetail(db, MapType::Page, raw, id);
	}
	
	Database::ID ImportMap::getPageMap(std::string raw){
		return getMapDetail(db, MapType::Page, raw);
	}
	
	void ImportMap::setFileMap(std::string raw, Database::ID id){
		setMapDetail(db, MapType::File, raw, id);
	}
	
	Database::ID ImportMap::getFileMap(std::string raw){
		return getMapDetail(db, MapType::File, raw);
	}
	
	void ImportMap::ImportMap::setThreadMap(std::string raw, Database::ID id){
        setMapDetail(db, MapType::Thread, raw, id);
	}
	
    Database::ID ImportMap::ImportMap::getThreadMap(std::string raw){
        return getMapDetail(db, MapType::Thread, raw);
    }
    
    void ImportMap::setCategoryMap(std::string raw, Database::ID id){
        setMapDetail(db, MapType::Category, raw, id);
    }
    
    Database::ID ImportMap::getCategoryMap(std::string raw){
        return getMapDetail(db, MapType::Category, raw);
    }
	
	namespace{
		uint64_t getTimeStamp(nlohmann::json time){
			return std::stoll(time.get<std::string>());
		}
	}
	
	void importFullArchive(Database* database, std::string archiveDirectory){
		Importer::ImportMap map(database);
		Importer::importBasicPageDataFromFolder(database, map, archiveDirectory + "pages/");
		
        //Importer::importForumGroups(database, map, Json::loadJsonFromFile(archiveDirectory + ""));
        //Importer::importThread(database, map, Json::loadJsonFromFile(archiveDirectory + ""));
        
        //Importer::performPageDataLinksFromFolder(database, map, archiveDirectory + "pages/");
        Importer::uploadPageFilesFromFolder(database, map, archiveDirectory + "pages/");
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
				try{
					importBasicPageData(database, map, Json::loadJsonFromFile(i->path().string() + "/data.json"));
				}
				catch(std::exception& e){
					std::cout << "ERROR!\n";
				}
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
	
	void performPageDataLinksFromFolder(Database* database, ImportMap& map, std::string pagesDirectory){
        for(boost::filesystem::directory_iterator i(pagesDirectory); i != boost::filesystem::directory_iterator(); i++){
			if(boost::filesystem::is_directory(i->path())){
				std::cout << "Linking " << i->path().string() << "\n";
				nlohmann::json pageJson = Json::loadJsonFromFile(i->path().string() + "/data.json");
				linkPageParent(database, map, pageJson);
				linkPageDiscussionThread(database, map, pageJson);
			}
		}
	}
	
	void linkPageParent(Database* database, ImportMap& map, nlohmann::json pageData){
		if(pageData["parent"].get<std::string>() != ""){
			database->setPageParent(map.getPageMap(pageData["id"].get<std::string>()), map.getPageMap(pageData["parent"].get<std::string>()));
		}
	}
	
	void linkPageDiscussionThread(Database* database, ImportMap& map, nlohmann::json pageData){
		
	}
	
	void uploadPageFilesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory){
        for(boost::filesystem::directory_iterator i(pagesDirectory); i != boost::filesystem::directory_iterator(); i++){
			if(boost::filesystem::is_directory(i->path())){
				std::cout << "Uploading files for " << i->path().string() << "\n";
                uploadPageFiles(database, map, Json::loadJsonFromFile(i->path().string() + "/data.json"), i->path().string());
			}
		}
	}
	
	void uploadPageFiles(Database* database, ImportMap& map, nlohmann::json pageData, std::string pageDirectory){
		for(nlohmann::json file : pageData["files"]){
			Database::ID fileId = map.getFileMap(file["id"].get<std::string>());
			std::ifstream fileStream(pageDirectory + "/files/" + file["id"].get<std::string>());
			database->uploadPageFile(fileId, fileStream);
		}
	}
}
