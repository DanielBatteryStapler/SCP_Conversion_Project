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
		
		struct PageVote{
			enum class Type{Unknown, Up, Down};
			std::optional<Database::ID> authorId;
			Type type = Type::Unknown;
		};
		
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
			std::optional<Database::ID> authorId;
			std::string description;
			TimeStamp timeStamp;
		};
		
		struct ForumGroup{
			std::string title;
			std::string description;
		};
		
		struct ForumCategory{
			std::string sourceId;
			std::string title;
			std::string description;
		};
		
		struct ForumThread{
			std::string sourceId;
			Database::ID parent;
			std::string title;
			std::optional<Database::ID> authorId;
			std::string description;
			TimeStamp timeStamp;
		};
		
		struct ForumPost{
			Database::ID parentThread;
			std::optional<Database::ID> parentPost;
			std::string title;
			std::optional<Database::ID> authorId;
			std::string content;
			TimeStamp timeStamp;
		};
		
		struct Author{
			enum class Type:short{User=0, System=1};
			Type type = Type::User;
			std::string name;
		};
		
		enum class MapCategory:short{Page=0, File=1, Author=2};
		
	private:
		Database() = default;
	public:
		Database(const Database&) = delete;
		~Database();
		
		static std::unique_ptr<Database> connectToDatabase(std::string databaseName);
		static void eraseDatabase(std::unique_ptr<Database>&& database);
		
		void cleanAndInitDatabase();
		
		void setIdMap(MapCategory category, std::string sourceId, Database::ID id);
		std::optional<Database::ID> getIdMap(MapCategory category, std::string sourceId);
		std::optional<std::string> getIdMapRaw(MapCategory category, Database::ID id);
		
		
		int64_t getNumberOfPages();
		
		std::optional<Database::ID> createPage(std::string name);
		void resetPage(Database::ID id, std::string name);
		std::optional<Database::ID> getPageId(std::string name);
		std::string getPageName(Database::ID id);
		
		std::vector<Database::ID> getPageList();
		
		std::optional<std::string> getPageDiscussion(Database::ID id);
		void setPageDiscussion(Database::ID id, std::optional<std::string> discussion);
		
		std::optional<std::string> getPageParent(Database::ID id);
		void setPageParent(Database::ID id, std::optional<std::string> parent);
		
		std::vector<std::string> getPageTags(Database::ID id);
		void setPageTags(Database::ID id, std::vector<std::string> tags);
		
		void addPageVote(Database::ID id, Database::PageVote vote);
		std::vector<Database::PageVote> getPageVotes(Database::ID id);
		std::int64_t countPageVotes(Database::ID id);
		
		
		int64_t getNumberOfPageRevisions();
		
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
		std::optional<Database::ID> getForumCategoryId(std::string sourceId);
		std::vector<Database::ID> getForumCategories(Database::ID group);
		
		
		int64_t getNumberOfForumThreads();
		
		Database::ID createForumThread(Database::ForumThread thread);
		void resetForumThread(Database::ID id, Database::ForumThread thread);
		Database::ForumThread getForumThread(Database::ID thread);
		std::optional<Database::ID> getForumThreadId(std::string sourceId);
		std::vector<Database::ID> getForumThreads(Database::ID category, std::int64_t count = 25, std::int64_t offset = 0);
		std::int64_t getNumberOfForumThreads(Database::ID category);
		
		
		int64_t getNumberOfForumPosts();
		
		Database::ID createForumPost(Database::ForumPost post);
		Database::ForumPost getForumPost(Database::ID post);
		std::vector<Database::ID> getForumReplies(Database::ID parentThread, std::optional<Database::ID> parentPost = {}, std::int64_t count = 25, std::int64_t offset = 0);
		std::int64_t getNumberOfForumReplies(Database::ID parentThread, std::optional<Database::ID> parentPost = {});
		
		Database::ID createAuthor(Database::Author author);
		void resetAuthor(Database::ID id, Database::Author author);
		Database::Author getAuthor(Database::ID id);
		
	private:
		soci::session sql;
};

#endif // DATABASE_HPP
