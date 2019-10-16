#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <optional>

#include <mongocxx/client.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

class Database{
	public:
		using ID = bsoncxx::oid;
		using DateTime = bsoncxx::types::b_date;
		
		struct PageRevision{
			std::string title;
			std::string sourceCode;
			struct MetaData{
				
			} metaData;
			std::string changeMessage;
		};
		
	private:
		const std::string colId = "_id";//this one is always an index
		
		const std::string pagesCol = "pages";
		const std::string pagesColName = "name";//this one is an index
		const std::string pagesColRevisions = "revisions";
		
		const std::string revisionCol = "revisions";
		const std::string revisionColTitle = "title";
		const std::string revisionColSourceCode = "sourceCode";
		const std::string revisionColChangeMessage = "changeMessage";
		
		Database() = default;
	public:
		Database(const Database&) = delete;
		~Database();
		
		static std::unique_ptr<Database> connectToMongoDatabase(std::string databaseName);
		static void wipeDatabaseFromMongo(std::unique_ptr<Database>&& db);
		void cleanAndInitDatabase();
		
		int64_t getNumberOfPages();
		
		std::optional<Database::ID> createPage(std::string name);
		std::optional<Database::ID> getPageId(std::string name);
		
		Database::ID createPageRevision(Database::ID page, Database::PageRevision revision);
		Database::PageRevision getPageRevision(Database::ID revision);
		Database::PageRevision getLatestPageRevision(Database::ID page);
		std::vector<Database::ID> getPageRevisions(Database::ID page);
		
	private:
		mongocxx::client dbClient;
		mongocxx::database database;
		mongocxx::gridfs::bucket gridfs;
};

std::ostream& operator<<(std::ostream &out, const Database::ID &c);

#endif // DATABASE_H
