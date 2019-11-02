#include <iostream>

#include "Tests/Tests.hpp"

#include <mongocxx/instance.hpp>

#include "Website/Website.hpp"
#include "Database/Database.hpp"
#include "Database/Importer.hpp"

#include "Config.hpp"

int main(int argc, char** argv){
	
	mongocxx::instance instance{};//this needs to exist for the entire program so mongodb works
	
	if(argc == 2 && std::string(argv[1]) == "--runTests"){
		Tests::runAllTests();
	}
	if(argc == 2 && std::string(argv[1]) == "--importPages"){
		std::unique_ptr<Database> database = Database::connectToMongoDatabase(Config::getProductionDatabaseName());
		database->cleanAndInitDatabase();
		Importer::ImportMap map;
		Importer::importBasicPageDataFromFolder(database.get(), map, "/home/daniel/File Collections/scpArchive/fullArchive/pages/");
	}
	if(argc == 2 && std::string(argv[1]) == "--runWebsite"){
		Website::run();
	}
	else{
		std::cout << "Invalid/No Options Specified\n";
	}
	
	return 0;
}
