#include "Database.hpp"

#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/string/to_string.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include "Json.hpp"

Database::~Database(){
	
}

namespace{
	using namespace Json;
}

std::unique_ptr<Database> Database::connectToMongoDatabase(std::string databaseName){
	std::unique_ptr<Database> output{new Database{}};//kinda weird to use a ``new`` with a unique_ptr, but it works and it's safe
	output->dbClient = mongocxx::client{mongocxx::uri{}};
	output->database = output->dbClient.database(databaseName);
	output->gridfs = output->database.gridfs_bucket();
	return output;
}

void Database::wipeDatabaseFromMongo(std::unique_ptr<Database>&& db){
	db->database.drop();
}

void Database::cleanAndInitDatabase(){
	nlohmann::json collections = fromBson(database.run_command(toBson({{"listCollections", 1}, {"nameOnly", true}})))["cursor"]["firstBatch"];
	for(auto& name : collections){
        database[name["name"].get<std::string>()].drop();
	}
	
	database[pagesCol].create_index(toBson({{pagesColName, 1}}), toBson({{"unique", 1}}));
	database[pageFilesCol].create_index(toBson({{pageFilesColName, 1}, {pageFilesColPageId, 1}}), toBson({{"unique", 1}}));
}

int64_t Database::getNumberOfPages(){
	mongocxx::pipeline p{};
	p.match(toBson({}));
	p.group(toBson({{"_id", 0}, {"n", {{"$sum", 1}}}}));
	auto cursor = database[pagesCol].aggregate(p);
	if(cursor.begin() == cursor.end()){
        return 0;
	}
	return fromBson(*cursor.begin())["n"].get<int>();
}

std::optional<Database::ID> Database::createPage(std::string name){
	try{
		auto result = 
		database[pagesCol].insert_one(toBson(
			{
				{pagesColName, name},
				{pagesColRevisions, 
					nlohmann::json::array()
				},
				{pagesColParent, nlohmann::json()},
				{pagesColDiscussion, nlohmann::json()},
				{pagesColTags,
                    nlohmann::json::array()
                }
			}
		));
		
		return result->inserted_id().get_oid().value;
	}
	catch(mongocxx::exception& e){
		//if there was an exception that probably means that there is already a page with that name
		//in the database already, so we should just return an empty optional, signaling that
		return {};
	}
}

std::optional<Database::ID> Database::getPageId(std::string name){
	auto result = database[pagesCol].find_one(toBson({{pagesColName, name}}));
	
	if(result){
        return getOid(fromBson(*result)[colId]);
	}
	else{
        return {};
	}
}

std::string Database::getPageName(Database::ID id){
	auto result = database[pagesCol].find_one(toBson({{colId, oid(id)}}));
    
    auto json = fromBson(*result);
    
    return json[pagesColName].get<std::string>();
}

std::vector<Database::ID> Database::getPageList(){
	auto result = database[pagesCol].find(toBson({}));
	
	std::vector<Database::ID> output;
	for(auto i : result){
		output.push_back(getOid(fromBson(i)[colId]));
	}
	
	return output;
}

std::optional<Database::ID> Database::getPageDiscussion(Database::ID id){
    auto result = database[pagesCol].find_one(toBson({{colId, oid(id)}}));
    
    auto json = fromBson(*result);
    
    if(json.find(pagesColDiscussion) == json.end()){
        return {};
    }
    else{
        return getOid(json[pagesColDiscussion]);
    }
}

void Database::setPageDiscussion(Database::ID id, std::optional<Database::ID> discussion){
    if(discussion){
        database[pagesCol].find_one_and_update(toBson({{colId, oid(id)}}), toBson({{"$set", {{pagesColDiscussion, oid(*discussion)}}}}));
    }
    else{
        database[pagesCol].find_one_and_update(toBson({{colId, oid(id)}}), toBson({{"$unset", {{pagesColDiscussion, ""}}}}));
    }
}

std::optional<Database::ID> Database::getPageParent(Database::ID id){
    auto result = database[pagesCol].find_one(toBson({{colId, oid(id)}}));
    
    auto json = fromBson(*result);
    
    if(json.find(pagesColParent) == json.end()){
        return {};
    }
    else{
        return getOid(json[pagesColParent]);
    }
}

void Database::setPageParent(Database::ID id, std::optional<Database::ID> parent){
    if(parent){
        database[pagesCol].find_one_and_update(toBson({{colId, oid(id)}}), toBson({{"$set", {{pagesColParent, oid(*parent)}}}}));
    }
    else{
        database[pagesCol].find_one_and_update(toBson({{colId, oid(id)}}), toBson({{"$unset", {{pagesColParent, ""}}}}));
    }
}

std::vector<std::string> Database::getPageTags(Database::ID id){
    auto result = database[pagesCol].find_one(toBson({{colId, oid(id)}}));
    
    auto json = fromBson(*result);
    
    return json[pagesColTags];
}

void Database::setPageTags(Database::ID id, std::vector<std::string> tags){
    database[pagesCol].find_one_and_update(toBson({{colId, oid(id)}}), toBson({{"$set", {{pagesColTags, tags}}}}));
}

Database::ID Database::createPageRevision(Database::ID page, Database::PageRevision revision){
	
	nlohmann::json doc{
		{revisionsColTitle, revision.title},
		{revisionsColTimeStamp, revision.timeStamp},
		{revisionsColChangeMessage, revision.changeMessage},
		{revisionsColChangeType, revision.changeType},
		{revisionsColSourceCode, revision.sourceCode}
	};
	
	if(revision.authorId){
		doc[revisionsColAuthorId] = oid(*revision.authorId);
	}
	
	auto result =
	database[revisionsCol].insert_one(
		toBson(doc)
	);
	
	Database::ID rev = result->inserted_id().get_oid().value;
	
	database[pagesCol].update_one(toBson({{colId, oid(page)}}), toBson({{"$push", {{pagesColRevisions, oid(rev)}}}}));
	
	return rev;
}

Database::PageRevision Database::getPageRevision(Database::ID revision){
	Database::PageRevision page;
	
	auto result = database[revisionsCol].find(toBson({{colId, oid(revision)}}));
	auto doc = fromBson(*result.begin());
	
	page.title = doc[revisionsColTitle].get<std::string>();
	if(doc.find(revisionsColAuthorId) != doc.end()){
		page.authorId = getOid(doc[revisionsColAuthorId]);
	}
	page.timeStamp = doc[revisionsColTimeStamp].get<TimeStamp>();
	page.changeMessage = doc[revisionsColChangeMessage].get<std::string>();
	page.changeType = doc[revisionsColChangeType].get<std::string>();
	page.sourceCode = doc[revisionsColSourceCode].get<std::string>();
	
	return page;
}

Database::PageRevision Database::getLatestPageRevision(Database::ID page){
	auto vec = getPageRevisions(page);
	return getPageRevision(vec.back());
}

std::vector<Database::ID> Database::getPageRevisions(Database::ID page){
	auto result = database[pagesCol].find(toBson({{colId, oid(page)}}));
	auto arr = fromBson(*result.begin())[pagesColRevisions];
	
	std::vector<Database::ID> output;
	for(auto i : arr){
		output.push_back(getOid(i));
	}
	
	return output;
}

std::optional<Database::ID> Database::createPageFile(Database::ID page, Database::PageFile file){
	nlohmann::json doc = {
		{pageFilesColPageId, oid(page)},
		{pageFilesColName, file.name},
		{pageFilesColDescription, file.description},
		{pageFilesColTimeStamp, file.timeStamp}
	};
	
	if(file.authorId){
		doc[pageFilesColAuthorId] = oid(*file.authorId);
	}
	
	try{
		auto result = 
		database[pageFilesCol].insert_one(toBson(doc));
		
		Database::ID fileId = result->inserted_id().get_oid().value;
		
		database[pagesCol].update_one(toBson({{colId, oid(page)}}), toBson({{"$push", {{pagesColFiles, oid(fileId)}}}}));
		
		return fileId;
	}
	catch(mongocxx::exception& e){
		//if there's an exception, assume that theres already a file with that name
		return {};
	}
}

std::optional<Database::ID> Database::getPageFileId(Database::ID page, std::string name){
	auto result = database[pageFilesCol].find_one(toBson({{pageFilesColPageId, oid(page)}, {pageFilesColName, name}}));
	
	if(result){
        return getOid(fromBson(*result)[colId]);
	}
	else{
        return {};
	}
}

Database::PageFile Database::getPageFile(Database::ID file){
	Database::PageFile output;
	
	auto result = database[pageFilesCol].find(toBson({{colId, oid(file)}}));
	auto doc = fromBson(*result.begin());
	
	output.name = doc[pageFilesColName].get<std::string>();
	output.description = doc[pageFilesColDescription].get<std::string>();
	output.timeStamp = doc[pageFilesColTimeStamp].get<TimeStamp>();
	if(doc.find(pageFilesColAuthorId) != doc.end()){
		output.authorId = getOid(doc[pageFilesColAuthorId]);
	}
	
	return output;
}

std::vector<Database::ID> Database::getPageFiles(Database::ID page){
	auto result = database[pagesCol].find(toBson({{colId, oid(page)}}));
	auto arr = fromBson(*result.begin())[pagesColFiles];
	
	std::vector<Database::ID> output;
	for(auto i : arr){
		output.push_back(getOid(i));
	}
	
	return output;
}

Database::ID Database::createForumGroup(Database::ForumGroup group){
	auto result = 
	database[forumGroupsCol].insert_one(toBson(
		{
			{forumGroupsColTitle, group.title},
			{forumGroupsColDescription, group.description},
			{forumGroupsColCategories, 
				nlohmann::json::array()
			}
		}
	));
	
	return result->inserted_id().get_oid().value;
}

Database::ForumGroup Database::getForumGroup(Database::ID group){
	Database::ForumGroup output;
	
	auto result = database[forumGroupsCol].find(toBson({{colId, oid(group)}}));
	auto doc = fromBson(*result.begin());
	
	output.title = doc[forumGroupsColTitle].get<std::string>();
	output.description = doc[forumGroupsColDescription].get<std::string>();
	
	return output;
}

std::vector<Database::ID> Database::getForumGroups(){
	auto result = database[forumGroupsCol].find(toBson({}));
	
	std::vector<Database::ID> output;
	for(auto i : result){
		output.push_back(getOid(fromBson(i)[colId]));
	}
	
	return output;
}

Database::ID Database::createForumCategory(Database::ID group, Database::ForumCategory category){
	auto result = 
	database[forumCategoriesCol].insert_one(toBson(
		{
			{forumCategoriesColTitle, category.title},
			{forumCategoriesColDescription, category.description}
		}
	));
	
	Database::ID categoryId = result->inserted_id().get_oid().value;
	
	database[forumGroupsCol].update_one(toBson({{colId, oid(group)}}), toBson({{"$push", {{forumGroupsColCategories, oid(categoryId)}}}}));
	
	return categoryId;
}

Database::ForumCategory Database::getForumCategory(Database::ID category){
	Database::ForumCategory output;
	
	auto result = database[forumCategoriesCol].find(toBson({{colId, oid(category)}}));
	auto doc = fromBson(*result.begin());
	
	output.title = doc[forumCategoriesColTitle].get<std::string>();
	output.description = doc[forumCategoriesColDescription].get<std::string>();
	
	return output;
}

std::vector<Database::ID> Database::getForumCategories(Database::ID group){
	auto result = database[forumGroupsCol].find(toBson({{colId, oid(group)}}));
	auto arr = fromBson(*result.begin())[forumGroupsColCategories];
	
	std::vector<Database::ID> output;
	for(auto i : arr){
		output.push_back(getOid(i));
	}
	
	return output;
}

std::ostream& operator<<(std::ostream &out, const Database::ID &c){
	out << c.to_string();
	return out;
}









