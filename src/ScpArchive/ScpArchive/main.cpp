#include <iostream>

#include "Tests/Tests.hpp"

#include <mongocxx/instance.hpp>

#include "Website/Website.hpp"
#include "Database/Database.hpp"
#include "Database/Importer.hpp"

#include "Config.hpp"

#include "UntestedUtils.hpp"

int main(int argc, char** argv){
	
	mongocxx::instance instance{};//this needs to exist for the entire program so mongodb works
	
	if(argc == 2 && std::string(argv[1]) == "--runTests"){
		Tests::runAllTests();
	}
	else if(argc == 2 && std::string(argv[1]) == "--importPages"){
		std::cout << "Are you sure you want to overwrite database \"" << Config::getProductionDatabaseName() << "\"?(y/n):\n";
		std::string temp;
		std::getline(std::cin, temp);
		if(temp != "y"){
			std::cout << "Aborting.\n";
			return 0;
		}
		
		std::unique_ptr<Database> database = Database::connectToMongoDatabase(Config::getProductionDatabaseName());
		database->cleanAndInitDatabase();
		Importer::ImportMap map;
		Importer::importBasicPageDataFromFolder(database.get(), map, "/home/daniel/File Collections/scpArchive/fullArchive/pages/");
	}
	else if(argc == 2 && std::string(argv[1]) == "--runWebsite"){
		Website::run();
	}
	else if(argc == 2 && std::string(argv[1]) == "--exportNodeDiagramData"){
		UntestedUtils::exportNodeDiagramData();
	}
	else{
		std::cout << "Invalid/No Options Specified\n";
	}
	
	return 0;
}
