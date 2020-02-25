#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <memory>
#include <optional>
#include <vector>
#include <soci/soci.h>

#include "../Config.hpp"

class Database{
	public:
		using ID = std::int64_t;
		
		struct PageRevision{
			std::string title;
			//author not implemented
			TimeStamp timeStamp;
			std::string changeMessage;
			std::string changeType;
			std::string sourceCode;
		};
		
		struct PageFile{
			std::string name;
			//author not implemented
			std::string description;
			TimeStamp timeStamp;
		};
		
		struct ForumGroup{
			std::string title;
			std::string description;
		};
		
		struct ForumCategory{
			std::string title;
			std::string description;
		};
		
		struct ForumThread{
			Database::ID parent;
			std::string title;
			//author not implemented
			std::string description;
			TimeStamp timeStamp;
		};
		
		struct ForumPost{
			Database::ID parentThread;
			std::optional<Database::ID> parentPost;
			std::string title;
			//author not implemented
			std::string content;
			TimeStamp timeStamp;
		};
		
		enum class MapType:short{Page=0, File=1, Thread=2, Category=3};
		
	private:
		Database() = default;
	public:
		Database(const Database&) = delete;
		~Database();
		
		static std::unique_ptr<Database> connectToDatabase(std::string databaseName);
		static void eraseDatabase(std::unique_ptr<Database>&& database);
		
		void cleanAndInitDatabase();
		
		void setIdMap(short category, std::string sourceId, Database::ID id);
		std::optional<Database::ID> getIdMap(short category, std::string sourceId);
		std::optional<std::string> getIdMapRaw(short category, Database::ID id);
		bool idMapExists(short category, std::string sourceId);
		
		int64_t getNumberOfPages();
		
		std::optional<Database::ID> createPage(std::string name);
		void resetPage(Database::ID id, std::string name);
		std::optional<Database::ID> getPageId(std::string name);
		std::string getPageName(Database::ID id);
		
		std::vector<Database::ID> getPageList();
		
		std::optional<Database::ID> getPageDiscussion(Database::ID id);
		void setPageDiscussion(Database::ID id, std::optional<Database::ID> discussion);
		std::optional<std::string> getPageParent(Database::ID id);
		void setPageParent(Database::ID id, std::optional<std::string> parent);
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
		void uploadPageFile(Database::ID file, std::istream& stream);
		void downloadPageFile(Database::ID file, std::ostream& stream);
		
		Database::ID createForumGroup(Database::ForumGroup group);
		Database::ForumGroup getForumGroup(Database::ID group);
		std::vector<Database::ID> getForumGroups();
		
		Database::ID createForumCategory(Database::ID group, Database::ForumCategory category);
		Database::ForumCategory getForumCategory(Database::ID category);
		std::vector<Database::ID> getForumCategories(Database::ID group);
		
		Database::ID createForumThread(Database::ForumThread thread);
		void resetForumThread(Database::ID id, Database::ForumThread thread);
		Database::ForumThread getForumThread(Database::ID thread);
		std::vector<Database::ID> getForumThreads(Database::ID category, std::int64_t count = 25, std::int64_t offset = 0);
		
		Database::ID createForumPost(Database::ForumPost post);
		Database::ForumPost getForumPost(Database::ID post);
		std::vector<Database::ID> getForumReplies(Database::ID parentThread, std::optional<Database::ID> parentPost = {});
		
	private:
		soci::session sql;
};

#endif // DATABASE_HPP
