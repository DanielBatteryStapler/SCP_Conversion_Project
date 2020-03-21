#include "Config.hpp"

namespace Config{
	std::string getTestingDatabaseName(){
		return "Temporary_SCP_Testing_Database";
	}
    
	unsigned int getNumberOfThreadsForGateway(){
		return 16;
	}
	
	std::string	getProductionDatabaseName(){
		return "SCPArchive";
	}
	
	std::string getFastCGISocket(){
		return ":8222";
	}
	
	std::string getWebsiteDomainName(){
		return "website.cloud.karagory.com";
	}
	
	std::string getDatabaseUser(){
		return "scparchive";
	}
	
	std::string getDatabasePassword(){
		return "password";
	}
	
	std::string getScraperFolder(){
		return "/home/daniel/File Collections/scpArchive/newBatchArchive/";
	}
}
