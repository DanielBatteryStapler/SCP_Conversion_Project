#include "Database.hpp"

#include <soci/mysql/soci-mysql.h>

using soci::use;
using soci::into;

#include "../Config.hpp"
#include "Json.hpp"

Database::~Database(){
	
}

namespace{
	using namespace Json;
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
	sql << "DROP TABLE IF EXISTS forumCategories";
	sql << "DROP TABLE IF EXISTS forumGroups";
	sql << "DROP TABLE IF EXISTS idMap";
	
	//recreate tables
	//sql << "SET NAMES utf8mb4";
	
	sql <<
	"CREATE TABLE pages( \n"
		"id BIGINT NOT NULL AUTO_INCREMENT, \n"
			"PRIMARY KEY (id), \n"
		"name TEXT NOT NULL, \n"
			"UNIQUE (name(255)), \n"
		"parent BIGINT DEFAULT NULL, \n"
			"FOREIGN KEY (parent) REFERENCES pages(id) ON DELETE SET NULL, \n"
		"discussion BIGINT DEFAULT NULL \n"//not implemented yet
			//should be a foreign key here
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
		"name TEXT NOT NULL, \n"
			"UNIQUE (name(255)), \n"
		"description TEXT NOT NULL, \n"
		"timeStamp BIGINT NOT NULL \n"
		//author not implemented
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
		//author not implemented
		"timeStamp BIGINT NOT NULL, \n"
		"changeMessage TEXT NOT NULL, \n"
		"changeType TEXT NOT NULL, \n"
		"sourceCode LONGTEXT NOT NULL \n"
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
		"parent BIGINT NOT NULL, \n"
			"FOREIGN KEY (parent) REFERENCES forumGroups(id) ON DELETE CASCADE, \n"
		"title TEXT NOT NULL, \n"
		"description TEXT NOT NULL \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
	sql << 
	"CREATE TABLE idMap( \n"
		"category TINYINT NOT NULL, \n"
		"id BIGINT NOT NULL, \n"
			"PRIMARY KEY (category, id), \n"
		"sourceId TEXT NOT NULL, \n"
			"UNIQUE (category, sourceId(255)) \n"
	")ENGINE=InnoDB CHARSET=utf8";
	
}

void Database::setIdMap(short category, std::string sourceId, Database::ID id){
	sql << "INSERT INTO idMap(category, id, sourceId) VALUES(:category, :id, :sourceId) ON DUPLICATE KEY UPDATE id=:id",
		use(category), use(id), use(sourceId), use(id);
}

std::optional<Database::ID> Database::getIdMap(short category, std::string sourceId){
	Database::ID id;
	sql << "SELECT id FROM idMap WHERE category=:category AND sourceId=:sourceID",
		use(category), use(sourceId), into(id);
	if(sql.got_data()){
		return id;
	}
	else{
		return {};
	}
}

int64_t Database::getNumberOfPages(){
	std::size_t pageCount;
	sql << "SELECT COUNT(*) FROM pages", into(pageCount);
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

std::optional<Database::ID> Database::getPageDiscussion(Database::ID id){
    Database::ID output;
    soci::indicator ind;
    sql << "SELECT discussion FROM pages WHERE id = :id", use(id), into(output, ind);
    if(ind == soci::i_null){
		return {};
    }
    else{
		return output;
    }
}

void Database::setPageDiscussion(Database::ID id, std::optional<Database::ID> discussion){
    if(discussion){
		sql << "UPDATE pages SET discussion=:discussion WHERE id=:id", use(*discussion), use(id);
    }
    else{
		sql << "UPDATE pages SET discussion=NULL WHERE id=:id", use(id);
    }
}

std::optional<Database::ID> Database::getPageParent(Database::ID id){
    Database::ID output;
    soci::indicator ind;
    sql << "SELECT parent FROM pages WHERE id = :id", use(id), into(output, ind);
    if(ind == soci::i_null){
		return {};
    }
    else{
		return output;
    }
}

void Database::setPageParent(Database::ID id, std::optional<Database::ID> parent){
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
	sql << "INSERT INTO revisions(page, title, timeStamp, changeMessage, changeType, sourceCode)"
	"VALUES(:page, :title, :timeStamp, :changeMessage, :changeType, :sourceCode)",
		use(page), use(revision.title), use(revision.timeStamp), use(revision.changeMessage), use(revision.changeType), use(revision.sourceCode);
	Database::ID revisionId;
	sql << "SELECT LAST_INSERT_ID()", into(revisionId);
	return revisionId;
}

Database::PageRevision Database::getPageRevision(Database::ID revision){
	Database::PageRevision page;
	
	sql << "SELECT title, timeStamp, changeMessage, changeType, sourceCode FROM revisions WHERE id=:id",
		use(revision), into(page.title), into(page.timeStamp), into(page.changeMessage), into(page.changeType), into(page.sourceCode);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
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
		sql << "INSERT INTO pageFiles(page, name, description, timeStamp) VALUES(:page, :name, :description, :timeStamp)",
			use(page), use(file.name), use(file.description), use(file.timeStamp);
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
	sql << "SELECT name, description, timestamp FROM pageFiles WHERE id=:id",
		use(file), into(pageFile.name), into(pageFile.description), into(pageFile.timeStamp);
	if(sql.got_data() == false){
		throw std::runtime_error("Cannot find SQL data");
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
	sql << "INSERT INTO forumCategories(title, description, parent) VALUES(:title, :description, :parent)",
		use(category.title), use(category.description), use(group);
	Database::ID id;
	sql << "SELECT LAST_INSERT_ID()", into(id);
	return id;
}

Database::ForumCategory Database::getForumCategory(Database::ID category){
	Database::ForumCategory forumCategory;
	sql << "SELECT title, description FROM forumCategories WHERE id=:id",
		use(category), into(forumCategory.title), into(forumCategory.description);
	return forumCategory;
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









