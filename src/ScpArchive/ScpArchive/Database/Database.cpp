#include "Database.hpp"

#include <mongocxx/exception/exception.hpp>
#include <bsoncxx/string/to_string.hpp>

namespace bbb = bsoncxx::builder::basic;

Database::~Database(){
	
}

namespace{
	//just a helper function for getting a string
	inline std::string getString(bsoncxx::document::element ele){
		return bsoncxx::string::to_string(ele.get_utf8().value);
	}
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
	std::vector<std::string> collections = database.list_collection_names();
	for(std::string name : collections){
		database[name].drop();
	}
	
	database[pagesCol].create_index(bbb::make_document(bbb::kvp(pagesColName, 1)), bbb::make_document(bbb::kvp("unique", 1)));
	
}

int64_t Database::getNumberOfPages(){
	return database[pagesCol].count_documents({});
}

std::optional<Database::ID> Database::createPage(std::string name){

	try{
		auto result = 
		database[pagesCol].insert_one(
			bbb::make_document(
				bbb::kvp(pagesColName, name),
				bbb::kvp(pagesColRevisions,
					bbb::make_array()
				)
			)
		);
		
		return result->inserted_id().get_oid().value;
	}
	catch(mongocxx::exception& e){
		//if there was an exception that probably means that there is already a page with that name
		//in the database already, so we should just return an empty optional, signaling that
		return {};
	}
}


std::optional<Database::ID> Database::getPageId(std::string name){
	auto result = database[pagesCol].find(bbb::make_document(bbb::kvp(pagesColName, name)));
	
	if(result.begin() == result.end()){
		return {};
	}
	else{
		auto doc = *result.begin();
		return doc[colId].get_oid().value;
	}
}

Database::ID Database::createPageRevision(Database::ID page, Database::PageRevision revision){
	
	auto result =
	database[revisionCol].insert_one(
		bbb::make_document(
			bbb::kvp(revisionColTitle, revision.title),
			bbb::kvp(revisionColSourceCode, revision.sourceCode),
			bbb::kvp(revisionColChangeMessage, revision.changeMessage)
		)
	);
	
	Database::ID rev = result->inserted_id().get_oid().value;
	
	database[pagesCol].update_one(bbb::make_document(bbb::kvp(colId, page)), bbb::make_document(bbb::kvp("$push", bbb::make_document(bbb::kvp(pagesColRevisions, rev)))));
	
	return rev;
}

Database::PageRevision Database::getPageRevision(Database::ID revision){
	Database::PageRevision page;
	
	auto result = database[revisionCol].find(bbb::make_document(bbb::kvp(colId, revision)));
	auto doc = *result.begin();
	
	page.title = getString(doc[revisionColTitle]);
	page.sourceCode = getString(doc[revisionColSourceCode]);
	page.changeMessage = getString(doc[revisionColChangeMessage]);
	
	return page;
}

Database::PageRevision Database::getLatestPageRevision(Database::ID page){
	auto vec = getPageRevisions(page);
	return getPageRevision(vec.back());
}

std::vector<Database::ID> Database::getPageRevisions(Database::ID page){
	
	auto result = database[pagesCol].find(bbb::make_document(bbb::kvp(colId, page)));
	auto arr = (*result.begin())[pagesColRevisions].get_array().value;
	
	std::vector<Database::ID> output;
	for(auto i : arr){
		output.push_back(i.get_oid().value);
	}
	
	return output;
}

std::ostream& operator<<(std::ostream &out, const Database::ID &c){
	out << c.to_string();
	return out;
}









