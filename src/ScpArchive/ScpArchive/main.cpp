#include <iostream>

#include "Tests/Tests.hpp"

#include "Website/Website.hpp"
#include "Database/Database.hpp"
#include "Database/Batch.hpp"

#include "Config.hpp"

#include "UntestedUtils.hpp"

#include "Parser/Parser.hpp"
#include "Parser/Treer.hpp"
#include "Parser/HTMLConverter.hpp"

#include "RuleSet.hpp"

int main(int argc, char** argv){
	if(argc == 2 && std::string(argv[1]) == "--runCustomTest"){
        Parser::printFullRuleSetList();
	}
	else if(argc == 2 && std::string(argv[1]) == "--runTests"){
		Tests::runAllTests();
	}
	else if(argc == 2 && std::string(argv[1]) == "--clearData"){
        std::cout << "Are you sure you want to CLEAR database \"" << Config::getProductionDatabaseName() << "\"?(y/n):\n";
		std::string temp;
		std::getline(std::cin, temp);
		if(temp != "y"){
			std::cout << "Aborting.\n";
			return 0;
		}
		
		std::unique_ptr<Database> database = Database::connectToDatabase(Config::getProductionDatabaseName());
		database->cleanAndInitDatabase();
	}
	else if(argc == 2 && std::string(argv[1]) == "--doBatches"){
		std::string batchesFolder = Config::getScraperFolder() + "batches/";
		std::string batchDataFile = Config::getScraperFolder() + "batchData.json";
		Importer::handleBatches(batchesFolder, batchDataFile);
		
	}
	else if(argc == 2 && std::string(argv[1]) == "--autoDoBatches"){
		std::string batchesFolder = Config::getScraperFolder() + "batches/";
		std::string batchDataFile = Config::getScraperFolder() + "batchData.json";
		Importer::handleBatches(batchesFolder, batchDataFile, true);
	}
	else if(argc == 3 && std::string(argv[1]) == "--doCustomBatch"){
		Importer::importBatch(argv[2]);
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
