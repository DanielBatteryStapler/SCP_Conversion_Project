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
        
        void setAuthorMap(std::string raw, Database::ID id);
        Database::ID getAuthorMap(std::string raw);
        std::string getAuthorMapRaw(Database::ID id);
        bool authorMapExists(std::string raw);
        
	private:
		Database* db;
    };
	
	void importAuthors(Database* database, ImportMap& map, const nlohmann::json& authorsData);
	void importAuthor(Database* database, ImportMap& map, const nlohmann::json& authorData);
	
	void importForumGroups(Database* database, ImportMap& map, const nlohmann::json& forumGroups);
	void importThreadsFromFolder(Database* database, ImportMap& map, std::string threadsDirectory, std::vector<std::string> threads);
	void importPosts(Database* database, ImportMap& map, const nlohmann::json& posts, Database::ID parentThread, std::optional<Database::ID> parentPost);
	void importThread(Database* database, ImportMap& map, const nlohmann::json& threadData);
	
	void importPagesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages);
	void importPage(Database* database, ImportMap& map, const nlohmann::json& pageData);
	
	void uploadPageFilesFromFolder(Database* database, ImportMap& map, std::string pagesDirectory, std::vector<std::string> pages);
	void uploadPageFiles(Database* database, ImportMap& map, const nlohmann::json& pageData, std::string pageDirectory);
}

#endif // IMPORTER_HPP
