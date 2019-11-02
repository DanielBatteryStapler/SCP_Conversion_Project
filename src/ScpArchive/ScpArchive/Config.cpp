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
}
