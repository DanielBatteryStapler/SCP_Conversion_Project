#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <optional>

#include <mongocxx/client.hpp>
#include <bsoncxx/oid.hpp>

#include <optional>

class Database{
	public:
		using ID = bsoncxx::oid;
		using TimeStamp = int64_t;
		
		struct PageRevision{
			std::string title;
			std::optional<ID> authorId;
			TimeStamp timeStamp;
			std::string changeMessage;
			std::string changeType;
			std::string sourceCode;
		};
		
	private:
		const std::string colId = "_id";//this one is always an index
		
		const std::string pagesCol = "pages";
		const std::string pagesColName = "name";//this one is an index
		const std::string pagesColRevisions = "revisions";
		
		const std::string revisionCol = "revisions";
		const std::string revisionColTitle = "title";
		const std::string revisionColAuthorId = "authorId";
		const std::string revisionColTimeStamp = "timeStamp";
		const std::string revisionColChangeMessage = "changeMessage";
		const std::string revisionColChangeType = "changeType";
		const std::string revisionColSourceCode = "sourceCode";
		
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
