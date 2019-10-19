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
					{}
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
	auto result = database[pagesCol].find(toBson({{pagesColName, name}}));
	
	if(result.begin() == result.end()){
		return {};
	}
	else{
		auto doc = fromBson(*result.begin());
		return getOid(doc[colId]);
	}
}

Database::ID Database::createPageRevision(Database::ID page, Database::PageRevision revision){
	
	nlohmann::json doc{
		{revisionColTitle, revision.title},
		{revisionColTimeStamp, revision.timeStamp},
		{revisionColChangeMessage, revision.changeMessage},
		{revisionColChangeType, revision.changeType},
		{revisionColSourceCode, revision.sourceCode}
	};
	
	if(revision.authorId){
		doc[revisionColAuthorId] = oid(*revision.authorId);
	}
	
	auto result =
	database[revisionCol].insert_one(
		toBson(doc)
	);
	
	Database::ID rev = result->inserted_id().get_oid().value;
	
	database[pagesCol].update_one(toBson({{colId, oid(page)}}), toBson({{"$push", {{pagesColRevisions, oid(rev)}}}}));
	
	return rev;
}

Database::PageRevision Database::getPageRevision(Database::ID revision){
	Database::PageRevision page;
	
	auto result = database[revisionCol].find(toBson({{colId, oid(revision)}}));
	auto doc = fromBson(*result.begin());
	
	page.title = doc[revisionColTitle].get<std::string>();
	if(doc.find(revisionColAuthorId) != doc.end()){
		page.authorId = getOid(doc[revisionColAuthorId]);
	}
	page.timeStamp = doc[revisionColTimeStamp].get<TimeStamp>();
	page.changeMessage = doc[revisionColChangeMessage].get<std::string>();
	page.changeType = doc[revisionColChangeType].get<std::string>();
	page.sourceCode = doc[revisionColSourceCode].get<std::string>();
	
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

std::ostream& operator<<(std::ostream &out, const Database::ID &c){
	out << c.to_string();
	return out;
}









