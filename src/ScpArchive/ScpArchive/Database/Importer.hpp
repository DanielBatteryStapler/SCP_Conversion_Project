#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <string>
#include <map>

#include "Database.hpp"

#include <nlohmann/json.hpp>

namespace Importer{
    
    struct ImportMap{
		std::map<std::string, Database::ID> pageMap;
		std::map<std::string, Database::ID> fileMap;
        std::map<std::string, Database::ID> threadMap;
        std::map<std::string, Database::ID> categoryMap;
        
        void setPageMap(std::string raw, Database::ID id);
        Database::ID getPageMap(std::string raw);
        
        void setFileMap(std::string raw, Database::ID id);
        Database::ID getFileMap(std::string raw);
        
        void setThreadMap(std::string raw, Database::ID id);
        Database::ID getThreadMap(std::string raw);
        
        void setCategoryMap(std::string raw, Database::ID id);
        Database::ID getCategoryMap(std::string raw);
    };
    
	void importFullArchive(Database* database, std::string archiveDirectory);
	
	void importForumGroups(Database* database, ImportMap& map, nlohmann::json forumGroups);
	void importThread(Database* database, ImportMap& map, nlohmann::json threadData);
	
	void importBasicPageDataFromFolder(Database* database, ImportMap& map, std::string pagesDirectory);
	void importBasicPageData(Database* database, ImportMap& map, nlohmann::json pageData);
	
	void performPageDataLinksFromFolder(Database* database, ImportMap& map, std::string pagesDirectory);
	void linkPageParent(Database* database, ImportMap& map, nlohmann::json pageData);
	void linkPageDiscussionThread(Database* database, ImportMap& map, nlohmann::json pageData);
	
	void uploadPageFilesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory);
	void uploadPageFiles(Database* database, ImportMap& map, nlohmann::json pageData, std::string pageDirectory);
}

#endif // IMPORTER_HPP
