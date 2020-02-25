#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <string>
#include <map>

#include "Database.hpp"

#include <nlohmann/json.hpp>

namespace Importer{
    
    struct ImportMap{
	public:
		
		ImportMap(Database* database);
        
        void setPageMap(std::string raw, Database::ID id);
        Database::ID getPageMap(std::string raw);
        std::string getPageMapRaw(Database::ID id);
        bool pageMapExists(std::string raw);
        
        void setFileMap(std::string raw, Database::ID id);
        Database::ID getFileMap(std::string raw);
        std::string getFileMapRaw(Database::ID id);
        bool fileMapExists(std::string raw);
        
        void setThreadMap(std::string raw, Database::ID id);
        Database::ID getThreadMap(std::string raw);
        std::string getThreadMapRaw(Database::ID id);
        bool threadMapExists(std::string raw);
        
        void setCategoryMap(std::string raw, Database::ID id);
        Database::ID getCategoryMap(std::string raw);
        std::string getCategoryMapRaw(Database::ID id);
        bool categoryMapExists(std::string raw);
        
	private:
		Database* db;
    };
	
	void importForumGroups(Database* database, ImportMap& map, nlohmann::json forumGroups);
	void importThreadsFromFolder(Database* database, ImportMap& map, std::string threadsDirectory, std::vector<std::string> threads);
	void importPosts(Database* database, ImportMap& map, const nlohmann::json& posts, Database::ID parentThread, std::optional<Database::ID> parentPost);
	void importThread(Database* database, ImportMap& map, nlohmann::json threadData);
	
	void importBasicPageDataFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages);
	void importBasicPageData(Database* database, ImportMap& map, nlohmann::json pageData);
	
	void performPageDataLinksFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages);
	void linkPageParent(Database* database, ImportMap& map, nlohmann::json pageData);
	void linkPageDiscussionThread(Database* database, ImportMap& map, nlohmann::json pageData);
	
	void uploadPageFilesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages);
	void uploadPageFiles(Database* database, ImportMap& map, nlohmann::json pageData, std::string pageDirectory);
}

#endif // IMPORTER_HPP
