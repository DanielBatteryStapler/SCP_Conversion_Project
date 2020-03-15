#include "Database.hpp"

#include <soci/mysql/soci-mysql.h>

using soci::use;
using soci::into;

#include "../Config.hpp"

Database::~Database(){
	
}

std::unique_ptr<Database> Database::connectToDatabase(std::string databaseName){
	std::unique_ptr<Database> output{new Database{}};
	output->sql.open(soci::mysql, "dbname=" + databaseName + " user='" + Config::getDatabaseUser() + "' password='" + Config::getDatabasePassword() + "'");
	return output;
}

void Database::eraseDatabase(std::unique_ptr<Database>&& database){
	database->cleanAndInitDatabase();
	
	database.reset();//make sure to delete the database so it is not left in an invalid state
}

void Database::cleanAndInitDatabase(){
	//remove all tables
	sql << "DROP TABLE IF EXISTS pageTags";
	sql << "DROP TABLE IF EXISTS pageFileData";
	sql << "DROP TABLE IF EXISTS pageFiles";
	sql << "DROP TABLE IF EXISTS revisions";
	sql << "DROP TABLE IF EXISTS pages";
	sql << "DROP TABLE IF EXISTS forumPosts";
	sql << "DROP TABLE IF EXISTS forumThreads";
	sql << "DROP TABLE IF EXISTS forumCategories";
	sql << "DROP TABLE IF EXISTS forumGroups";
	sql << "DROP TABLE IF EXISTS authors";
	sql << "DROP TABLE IF EXISTS idMap";
	
	//recreate tables
	sql <<
	"CREATE TABLE authors( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"type TINYINT NOT NULL, \n"
		"name TEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE forumGroups( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"title TEXT NOT NULL, \n"
		"description TEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE forumCategories( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"sourceId TEXT NOT NULL, \n"
			"UNIQUE (sourceId(255)), \n"
		"parent BIGINT NOT NULL, \n"
			"FOREIGN KEY (parent) REFERENCES forumGroups(id) ON DELETE CASCADE, \n"
		"title TEXT NOT NULL, \n"
		"description TEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE forumThreads( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"sourceId TEXT NOT NULL, \n"
			"UNIQUE (sourceId(255)), \n"
		"parent BIGINT NOT NULL, \n"
			"FOREIGN KEY (parent) REFERENCES forumCategories(id) ON DELETE CASCADE, \n"
		"title TEXT NOT NULL, \n"
		"authorId BIGINT, \n"
			"FOREIGN KEY (authorId) REFERENCES authors(id) ON DELETE RESTRICT, \n"
		"description TEXT NOT NULL, \n"
		"timeStamp BIGINT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE forumPosts( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"parentThread BIGINT NOT NULL, \n"
			"FOREIGN KEY (parentThread) REFERENCES forumThreads(id) ON DELETE CASCADE, \n"
		"parentPost BIGINT, \n"
			"FOREIGN KEY (parentPost) REFERENCES forumPosts(id) ON DELETE CASCADE, \n"
		"title TEXT NOT NULL, \n"
		"authorId BIGINT, \n"
			"FOREIGN KEY (authorId) REFERENCES authors(id) ON DELETE RESTRICT, \n"
		"content TEXT NOT NULL, \n"
		"timeStamp BIGINT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE pages( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"name TEXT NOT NULL, \n"
			"UNIQUE (name(255)), \n"
		"parent TEXT DEFAULT NULL, \n"
		"discussion TEXT DEFAULT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE pageTags( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"page BIGINT NOT NULL, \n"
			"FOREIGN KEY (page) REFERENCES pages(id) ON DELETE CASCADE, \n"
		"tag TEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE pageFiles( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"page BIGINT NOT NULL, \n"
			"FOREIGN KEY (page) REFERENCES pages(id) ON DELETE CASCADE, \n"
		"name TEXT NOT NULL COLLATE utf8_bin, \n"
			"UNIQUE (page, name(255)), \n"
		"authorId BIGINT, \n"
			"FOREIGN KEY (authorId) REFERENCES authors(id) ON DELETE RESTRICT, \n"
		"description TEXT NOT NULL, \n"
		"timeStamp BIGINT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE pageFileData( \n"
		"pageFile BIGINT NOT NULL, \n"
			"PRIMARY KEY (pageFile), \n"
			"FOREIGN KEY (pageFile) REFERENCES pageFiles(id) ON DELETE CASCADE, \n"
		"data LONGBLOB NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql <<
	"CREATE TABLE revisions( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"page BIGINT NOT NULL, \n"
			"FOREIGN KEY (page) REFERENCES pages(id) ON DELETE CASCADE, \n"
		"title TEXT NOT NULL, \n"
		"authorId BIGINT, \n"
			"FOREIGN KEY (authorId) REFERENCES authors(id) ON DELETE RESTRICT, \n"
		"timeStamp BIGINT NOT NULL, \n"
		"changeMessage TEXT NOT NULL, \n"
		"changeType TEXT NOT NULL, \n"
		"sourceCode LONGTEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql << 
	"CREATE TABLE idMap( \n"
		"category TINYINT NOT NULL, \n"
		"id BIGINT NOT NULL, \n"
			"PRIMARY KEY (category, id), \n"
		"sourceId TEXT NOT NULL, \n"
			"UNIQUE (category, sourceId(255)) \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	
	{
		//make sure the "system" author exists
		Author system;
		system.name = "system";
		system.type = Author::Type::System;
		ID systemId = createAuthor(system);
		///TODO: go into ScpScraper and make it so when it outputs data it uses something like "system" for the id instead of "wikidot"
		//if we accidentally show this id to end users, it could seem like we're misrepresenting a trademark, which would be very bad
		setIdMap(MapCategory::Author, "wikidot", systemId);
	}
}

void Database::setIdMap(MapCategory category, std::string sourceId, Database::ID id){
	sql << "INSERT INTO idMap(category, id, sourceId) VALUES(:category, :id, :sourceId) ON DUPLICATE KEY UPDATE sourceId=:sourceId, id=:id",
		use(static_cast<short>(category)), use(id), use(sourceId), use(sourceId), use(id);
}

std::optional<Database::ID> Database::getIdMap(MapCategory category, std::string sourceId){
	Database::ID id;
	sql << "SELECT id FROM idMap WHERE category=:category AND sourceId=:sourceID",
		use(static_cast<short>(category)), use(sourceId), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

std::optional<std::string> Database::getIdMapRaw(MapCategory category, Database::ID id){
	std::string sourceId;
	sql << "SELECT sourceId FROM idMap WHERE category=:category AND id=:id",
		use(static_cast<short>(category)), use(id), into(sourceId);
	if(sql.got_data()){
		return sourceId;
	}
	else{
		return {};
	}
}

std::int64_t Database::getNumberOfPages(){
	std::int64_t pageCount;
	sql << "SELECT COUNT(id) FROM pages", into(pageCount);
	return pageCount;
}

std::optional<Database::ID> Database::createPage(std::string name){
	try{
		sql << "INSERT INTO pages(name) VALUES(:name)", use(name);
		Database::ID id;
		sql << "SELECT LAST_INSERT_ID()", into(id);
		return id;
	}
	catch(std::exception& e){
		//if there was an exception that probably means that there is already a page with that name
		//in the database already, so we should just return an empty optional, signaling that
		return {};
	}
}

void Database::resetPage(Database::ID id, std::string name){
	std::vector<Database::ID> files = getPageFiles(id);
	//sql << "DELETE FROM idMap WHERE category=:category AND id=:id", use(static_cast<short>(MapType::File)), use(id);
	sql << "DELETE FROM pageTags WHERE page=:page", use(id);
	sql << "DELETE FROM pageFiles WHERE page=:page", use(id);
	sql << "DELETE FROM revisions WHERE page=:page", use(id);
	sql << "UPDATE pages SET name=:name, parent=NULL WHERE id=:id", use(name), use(id);
}

std::optional<Database::ID> Database::getPageId(std::string name){
	Database::ID id;
	sql << "SELECT id FROM pages WHERE name = :name", use(name), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

std::string Database::getPageName(Database::ID id){
    std::string name;
    sql << "SELECT name FROM pages WHERE id = :id", use(id), into(name);
    if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
    return name;
}

std::vector<Database::ID> Database::getPageList(){
	Database::ID page;
	soci::statement stmt = (sql.prepare << "SELECT id FROM pages", into(page));
	stmt.execute();
	
	std::vector<Database::ID> pages;
	while(stmt.fetch()){
		pages.push_back(page);
	}
	return pages;
}

std::optional<std::string> Database::getPageDiscussion(Database::ID id){
    std::string output;
    soci::indicator ind;
    sql << "SELECT discussion FROM pages WHERE id = :id", use(id), into(output, ind);
    if(ind == soci::i_null){
		return {};
    }
    else{
		return output;
    }
}

void Database::setPageDiscussion(Database::ID id, std::optional<std::string> discussion){
    if(discussion){
		sql << "UPDATE pages SET discussion=:discussion WHERE id=:id", use(*discussion), use(id);
    }
    else{
		sql << "UPDATE pages SET discussion=NULL WHERE id=:id", use(id);
    }
}

std::optional<std::string> Database::getPageParent(Database::ID id){
    std::string output;
    soci::indicator ind;
    sql << "SELECT parent FROM pages WHERE id = :id", use(id), into(output, ind);
    if(ind == soci::i_null){
		return {};
    }
    else{
		return output;
    }
}

void Database::setPageParent(Database::ID id, std::optional<std::string> parent){
    if(parent){
		sql << "UPDATE pages SET parent=:parent WHERE id=:id", use(*parent), use(id);
    }
    else{
		sql << "UPDATE pages SET parent=NULL WHERE id=:id", use(id);
    }
}

std::vector<std::string> Database::getPageTags(Database::ID id){
    std::string tag;
    soci::statement stmt = (sql.prepare << "SELECT tag FROM pageTags WHERE page=:id", use(id), into(tag));
    stmt.execute();
    
    std::vector<std::string> tags;
    while(stmt.fetch()){
		tags.push_back(tag);
    }
    return tags;
}

void Database::setPageTags(Database::ID id, std::vector<std::string> tags){
	sql << "DELETE FROM pageTags WHERE page=:id", use(id);
	for(auto tag : tags){
		sql << "INSERT INTO pageTags(page, tag) VALUES(:page, :tag)", use(id), use(tag);
	}
}

Database::ID Database::createPageRevision(Database::ID page, Database::PageRevision revision){
	if(revision.authorId){
		sql << "INSERT INTO revisions(page, title, authorId, timeStamp, changeMessage, changeType, sourceCode)"
		"VALUES(:page, :title, :authorId, :timeStamp, :changeMessage, :changeType, :sourceCode)",
			use(page), use(revision.title), use(*revision.authorId), use(revision.timeStamp), use(revision.changeMessage), use(revision.changeType), use(revision.sourceCode);
	}
	else{
		sql << "INSERT INTO revisions(page, title, authorId, timeStamp, changeMessage, changeType, sourceCode)"
		"VALUES(:page, :title, NULL, :timeStamp, :changeMessage, :changeType, :sourceCode)",
			use(page), use(revision.title), use(revision.timeStamp), use(revision.changeMessage), use(revision.changeType), use(revision.sourceCode);
	}
	Database::ID revisionId;
	sql << "SELECT LAST_INSERT_ID()", into(revisionId);
	return revisionId;
}

Database::PageRevision Database::getPageRevision(Database::ID revision){
	Database::PageRevision page;
	Database::ID authorId;
	soci::indicator ind;
	sql << "SELECT title, authorId, timeStamp, changeMessage, changeType, sourceCode FROM revisions WHERE id=:id",
		use(revision), into(page.title), into(authorId, ind), into(page.timeStamp), into(page.changeMessage), into(page.changeType), into(page.sourceCode);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	if(ind == soci::i_null){
		page.authorId = {};
    }
    else{
		page.authorId = authorId;
    }
	return page;
}

Database::PageRevision Database::getLatestPageRevision(Database::ID page){
	///TODO: optimize this
	auto vec = getPageRevisions(page);
	return getPageRevision(vec.back());
}

std::vector<Database::ID> Database::getPageRevisions(Database::ID page){
	Database::ID revision;
	soci::statement stmt = (sql.prepare << "SELECT id FROM revisions WHERE page=:page ORDER BY timeStamp ASC", use(page), into(revision));
	stmt.execute();
	
	std::vector<Database::ID> revisions;
	while(stmt.fetch()){
		revisions.push_back(revision);
	}
	return revisions;
}

std::optional<Database::ID> Database::createPageFile(Database::ID page, Database::PageFile file){
	try{
		if(file.authorId){
			sql << "INSERT INTO pageFiles(page, name, authorId, description, timeStamp) VALUES(:page, :name, :authorId, :description, :timeStamp)",
				use(page), use(file.name), use(*file.authorId), use(file.description), use(file.timeStamp);
		}
		else{
			sql << "INSERT INTO pageFiles(page, name, authorId, description, timeStamp) VALUES(:page, :name, NULL, :description, :timeStamp)",
				use(page), use(file.name), use(file.description), use(file.timeStamp);
		}
		Database::ID id;
		sql << "SELECT LAST_INSERT_ID()", into(id);
		return id;
	}
	catch(std::exception& e){
		return {};
	}
}

std::optional<Database::ID> Database::getPageFileId(Database::ID page, std::string name){
	Database::ID id;
	sql << "SELECT id FROM pageFiles WHERE page=:page AND name=:name", use(page), use(name), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

Database::PageFile Database::getPageFile(Database::ID file){
	Database::PageFile pageFile;
	Database::ID authorId;
	soci::indicator ind;
	sql << "SELECT name, authorId, description, timestamp FROM pageFiles WHERE id=:id",
		use(file), into(pageFile.name), into(authorId, ind), into(pageFile.description), into(pageFile.timeStamp);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	if(ind == soci::i_null){
		pageFile.authorId = {};
	}
	else{
		pageFile.authorId = authorId;
	}
	return pageFile;
}

std::vector<Database::ID> Database::getPageFiles(Database::ID page){
	Database::ID pageFile;
	soci::statement stmt = (sql.prepare << "SELECT id FROM pageFiles WHERE page=:page", use(page), into(pageFile));
	stmt.execute();
	
	std::vector<Database::ID> pageFiles;
	while(stmt.fetch()){
		pageFiles.push_back(pageFile);
	}
	
	return pageFiles;
}

void Database::uploadPageFile(Database::ID file, std::istream& stream){
	std::string fileData{std::istreambuf_iterator<char>{stream}, {}};
	sql << "INSERT INTO pageFileData(pageFile, data) values(:file, :data)", use(file), use(fileData);
}

void Database::downloadPageFile(Database::ID file, std::ostream& stream){
	std::string fileData;
	sql << "SELECT data FROM pageFileData WHERE pageFile=:file", use(file), into(fileData);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	stream << fileData;
}

Database::ID Database::createForumGroup(Database::ForumGroup group){
	sql << "INSERT INTO forumGroups(title, description) VALUES(:title, :description)", use(group.title), use(group.description);
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	return id;
}

Database::ForumGroup Database::getForumGroup(Database::ID group){
	Database::ForumGroup forumGroup;
	sql << "SELECT title, description FROM forumGroups WHERE id=:id", use(group), into(forumGroup.title), into(forumGroup.description);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	return forumGroup;
}

std::vector<Database::ID> Database::getForumGroups(){
	Database::ID group;
	soci::statement stmt = (sql.prepare << "SELECT id FROM forumGroups", into(group));
	stmt.execute();
	
	std::vector<Database::ID> groups;
	while(stmt.fetch()){
		groups.push_back(group);
	}
	
	return groups;
}

Database::ID Database::createForumCategory(Database::ID group, Database::ForumCategory category){
	sql << "INSERT INTO forumCategories(title, description, parent, sourceId) VALUES(:title, :description, :parent, :sourceId)",
		use(category.title), use(category.description), use(group), use(category.sourceId);
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	return id;
}

Database::ForumCategory Database::getForumCategory(Database::ID category){
	Database::ForumCategory forumCategory;
	sql << "SELECT title, description, sourceId FROM forumCategories WHERE id=:id",
		use(category), into(forumCategory.title), into(forumCategory.description), into(forumCategory.sourceId);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	return forumCategory;
}

std::optional<Database::ID> Database::getForumCategoryId(std::string sourceId){
	Database::ID id;
	sql << "SELECT id FROM forumCategories WHERE sourceId = :sourceId", use(sourceId), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

std::vector<Database::ID> Database::getForumCategories(Database::ID group){
	Database::ID category;
	soci::statement stmt = (sql.prepare << "SELECT id FROM forumCategories WHERE parent=:group", use(group), into(category));
	stmt.execute();
	
	std::vector<Database::ID> categories;
	while(stmt.fetch()){
		categories.push_back(category);
	}
	
	return categories;
}

Database::ID Database::createForumThread(Database::ForumThread thread){
	if(thread.authorId){
		sql << "INSERT INTO forumThreads(parent, title, authorId, description, timeStamp, sourceId) VALUES(:parent, :title, :authorId, :description, :timeStamp, :sourceId)",
			use(thread.parent), use(thread.title), use(*thread.authorId), use(thread.description), use(thread.timeStamp), use(thread.sourceId);
	}
	else{
		sql << "INSERT INTO forumThreads(parent, title, authorId, description, timeStamp, sourceId) VALUES(:parent, :title, NULL, :description, :timeStamp, :sourceId)",
			use(thread.parent), use(thread.title), use(thread.description), use(thread.timeStamp), use(thread.sourceId);
	}
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	return id;
}

void Database::resetForumThread(Database::ID id, Database::ForumThread thread){
	sql << "DELETE FROM forumPosts WHERE parentThread=:id", use(id);
	if(thread.authorId){
		sql << "UPDATE forumThreads SET parent=:parent, title=:title, authorId=:authorId, description=:description, timestamp=:timestamp, sourceId=:sourceId WHERE id=:id",
			use(thread.parent), use(thread.title), use(*thread.authorId), use(thread.description), use(thread.timeStamp), use(thread.sourceId), use(id);
	}
	else{
		sql << "UPDATE forumThreads SET parent=:parent, title=:title, authorId=NULL, description=:description, timestamp=:timestamp, sourceId=:sourceId WHERE id=:id",
			use(thread.parent), use(thread.title), use(thread.description), use(thread.timeStamp), use(thread.sourceId), use(id);
	}
}

Database::ForumThread Database::getForumThread(Database::ID thread){
	Database::ForumThread forumThread;
	
	Database::ID authorId;
	soci::indicator ind;
	
	sql << "SELECT parent, title, authorId, description, timestamp, sourceId FROM forumThreads WHERE id=:id", use(thread),
		into(forumThread.parent), into(forumThread.title), into(authorId, ind), into(forumThread.description), into(forumThread.timeStamp), into(forumThread.sourceId);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	if(ind == soci::i_null){
		forumThread.authorId = {};
	}
	else{
		forumThread.authorId = authorId;
	}
	return forumThread;
}

std::optional<Database::ID> Database::getForumThreadId(std::string sourceId){
	Database::ID id;
	sql << "SELECT id FROM forumThreads WHERE sourceId = :sourceId", use(sourceId), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

std::vector<Database::ID> Database::getForumThreads(Database::ID category, std::int64_t count, std::int64_t offset){
	Database::ID thread;
	soci::statement stmt = (sql.prepare << "SELECT id FROM forumThreads WHERE parent=:category ORDER BY timestamp DESC LIMIT :offset, :count",
							use(category), into(thread), use(offset), use(count));
	stmt.execute();
	
	std::vector<Database::ID> threads;
	while(stmt.fetch()){
		threads.push_back(thread);
	}
	
	return threads;
}

std::int64_t Database::getNumberOfForumThreads(Database::ID category){
    std::int64_t threadCount;
	sql << "SELECT COUNT(id) FROM forumThreads WHERE parent=:category", use(category), into(threadCount);
	return threadCount;
}

Database::ID Database::createForumPost(Database::ForumPost post){
	if(post.parentPost){
		sql << "INSERT INTO forumPosts(parentThread, parentPost, title, authorId, content, timeStamp) VALUES(:parentThread, :parentPost, :title, NULL, :content, :timeStamp)",
			use(post.parentThread), use(*post.parentPost), use(post.title), use(post.content), use(post.timeStamp);
	}
	else{
		sql << "INSERT INTO forumPosts(parentThread, parentPost, title, authorId, content, timeStamp) VALUES(:parentThread, NULL, :title, NULL, :content, :timeStamp)",
			use(post.parentThread), use(post.title), use(post.content), use(post.timeStamp);
	}
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	if(post.authorId){
		sql << "UPDATE forumPosts SET authorId=:authorId WHERE id=:id",
			use(*post.authorId), use(id);
	}
	return id;
}

Database::ForumPost Database::getForumPost(Database::ID post){
	Database::ForumPost forumPost;
	Database::ID parentPost;
	soci::indicator ind_parent;
	Database::ID authorId;
	soci::indicator ind_author;
	sql << "SELECT parentThread, parentPost, title, authorId, content, timestamp FROM forumPosts WHERE id=:id", use(post),
		into(forumPost.parentThread), into(parentPost, ind_parent), into(forumPost.title), into(authorId, ind_author), into(forumPost.content), into(forumPost.timeStamp);
    if(ind_parent == soci::i_null){
		forumPost.parentPost = {};
    }
    else{
		forumPost.parentPost = parentPost;
    }
    if(ind_author == soci::i_null){
		forumPost.authorId = {};
    }
    else{
		forumPost.authorId = authorId;
    }
	return forumPost;
}

std::vector<Database::ID> Database::getForumReplies(Database::ID parentThread, std::optional<Database::ID> parentPost, std::int64_t count, std::int64_t offset){
	Database::ID post;
	std::optional<soci::statement> stmt;
	if(parentPost){
		stmt = (sql.prepare << "SELECT id FROM forumPosts WHERE parentThread=:thread AND parentPost=:post ORDER BY timeStamp ASC LIMIT :offset, :count",
                use(parentThread), use(*parentPost), use(offset), use(count), into(post));
	}
	else{
		stmt = (sql.prepare << "SELECT id FROM forumPosts WHERE parentThread=:thread AND parentPost IS NULL ORDER BY timeStamp ASC LIMIT :offset, :count",
                use(parentThread), use(offset), use(count), into(post));
	}
	stmt->execute();
	
	std::vector<Database::ID> posts;
	while(stmt->fetch()){
		posts.push_back(post);
	}
	
	return posts;
}

std::int64_t Database::getNumberOfForumReplies(Database::ID parentThread, std::optional<Database::ID> parentPost){
    std::int64_t postCount;
    if(parentPost){
        sql << "SELECT COUNT(id) FROM forumPosts WHERE parentThread=:parentThread AND parentPost=:parentPost", use(parentThread), use(*parentPost), into(postCount);
    }
    else{
        sql << "SELECT COUNT(id) FROM forumPosts WHERE parentThread=:parentThread AND parentPost IS NULL", use(parentThread), into(postCount);
    }
	return postCount;
}

Database::ID Database::createAuthor(Database::Author author){
	sql << "INSERT INTO authors(type, name) VALUES(:type, :name)",
			use(static_cast<short>(author.type)), use(author.name);
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	return id;
}

void Database::resetAuthor(Database::ID id, Database::Author author){
	sql << "UPDATE authors SET type=:type, name=:name WHERE id=:id",
			use(static_cast<short>(author.type)), use(author.name), use(id);
}

Database::Author Database::getAuthor(Database::ID id){
	Database::Author out;
	short type;
	sql << "SELECT type, name FROM authors WHERE id=:id",
		use(id), into(type), into(out.name);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
	}
	out.type = static_cast<Author::Type>(type);
	return out;
}





