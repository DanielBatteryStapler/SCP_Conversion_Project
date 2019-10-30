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
			std::optional<Database::ID> authorId;
			TimeStamp timeStamp;
			std::string changeMessage;
			std::string changeType;
			std::string sourceCode;
		};
		
		struct PageFile{
			std::string name;
			std::string description;
			TimeStamp timeStamp;
			std::optional<Database::ID> authorId;
		};
		
		struct ForumGroup{
			std::string title;
			std::string description;
		};
		
		struct ForumCategory{
			std::string title;
			std::string description;
		};
		
	private:
		const std::string colId = "_id";//this one is always an index
		
		const std::string pagesCol = "pages";
		const std::string pagesColName = "name";//this one is an index
		const std::string pagesColParent = "parent";
		const std::string pagesColDiscussion = "discussion";
		const std::string pagesColTags = "tags";
		const std::string pagesColRevisions = "revisions";
		const std::string pagesColFiles = "files";
		
		const std::string revisionsCol = "revisions";
		const std::string revisionsColTitle = "title";
		const std::string revisionsColAuthorId = "authorId";
		const std::string revisionsColTimeStamp = "timeStamp";
		const std::string revisionsColChangeMessage = "changeMessage";
		const std::string revisionsColChangeType = "changeType";
		const std::string revisionsColSourceCode = "sourceCode";
		
		const std::string pageFilesCol = "pageFiles";
		const std::string pageFilesColPageId = "pageId";
		const std::string pageFilesColName = "name";//compound index with pageId
		const std::string pageFilesColDescription = "description";
		const std::string pageFilesColTimeStamp = "timeStamp";
		const std::string pageFilesColAuthorId = "authorId";
		
		///not yet implemented
		const std::string forumGroupsCol = "forumGroups";
		const std::string forumGroupsColTitle = "title";
		const std::string forumGroupsColDescription = "description";
		const std::string forumGroupsColCategories = "categories";
		
		const std::string forumCategoriesCol = "forumCategories";
		const std::string forumCategoriesColTitle = "title";
		const std::string forumCategoriesColDescription = "description";
		
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
		
		std::optional<Database::ID> getPageDiscussion(Database::ID id);
		void setPageDiscussion(Database::ID id, std::optional<Database::ID> discussion);
		std::optional<Database::ID> getPageParent(Database::ID id);
		void setPageParent(Database::ID id, std::optional<Database::ID> parent);
		std::vector<std::string> getPageTags(Database::ID id);
		void setPageTags(Database::ID id, std::vector<std::string> tags);
		
		Database::ID createPageRevision(Database::ID page, Database::PageRevision revision);
		Database::PageRevision getPageRevision(Database::ID revision);
		Database::PageRevision getLatestPageRevision(Database::ID page);
		std::vector<Database::ID> getPageRevisions(Database::ID page);
		
		std::optional<Database::ID> createPageFile(Database::ID page, Database::PageFile file);
		std::optional<Database::ID> getPageFileId(Database::ID page, std::string name);
		Database::PageFile getPageFile(Database::ID file);
		std::vector<Database::ID> getPageFiles(Database::ID page);
		
		Database::ID createForumGroup(Database::ForumGroup group);
		Database::ForumGroup getForumGroup(Database::ID group);
		std::vector<Database::ID> getForumGroups();
		
		Database::ID createForumCategory(Database::ID group, Database::ForumCategory category);
		Database::ForumCategory getForumCategory(Database::ID category);
		std::vector<Database::ID> getForumCategories(Database::ID group);
		
	private:
		mongocxx::client dbClient;
		mongocxx::database database;
		mongocxx::gridfs::bucket gridfs;
};

std::ostream& operator<<(std::ostream &out, const Database::ID &c);

#endif // DATABASE_H
