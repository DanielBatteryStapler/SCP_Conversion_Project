#include <iostream>

#include "Tests/Tests.hpp"

#include <mongocxx/instance.hpp>

#include "Website/Website.hpp"
#include "Database/Database.hpp"
#include "Database/Importer.hpp"

#include "Config.hpp"

#include "UntestedUtils.hpp"

#include "Parser/Parser.hpp"
#include "Parser/Treer.hpp"
#include "Parser/HTMLConverter.hpp"

int main(int argc, char** argv){
	
	mongocxx::instance instance{};//this needs to exist for the entire program so mongodb works
	
	if(argc == 2 && std::string(argv[1]) == "--runCustomTest"){
       // Tests::runAllTests();
        
        std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getProductionDatabaseName());

        std::optional<Database::ID> pageId = db->getPageId("scp-2317");
        if(pageId){
            Database::PageRevision revision = db->getLatestPageRevision(*pageId);
            
            std::ofstream fileOut("test.html");
            {
                MarkupOutStream out(&fileOut);
                
                out << "<!DOCTYPE html><html><head><link rel='stylesheet' type='text/css' href='static/style.css'><meta charset='UTF-8'><title>"_AM
                << revision.title << "</title></head><body>"_AM;

                std::ofstream("raw.txt") << revision.sourceCode;
                
                Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode);
                for(const auto& tok : pageTokens.tokens){
                //std::cout << Parser::toString(tok) << "\n";
                }
                Parser::PageTree pageTree = Parser::makeTreeFromTokenedPage(pageTokens);
                Parser::convertPageTreeToHtml(out, pageTree);
                out << "</body></html>"_AM;
            }
            fileOut.close();
        }
        else{
            std::cout << "Cannot find page\n";
        }
        
	}
	else if(argc == 2 && std::string(argv[1]) == "--runTests"){
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
