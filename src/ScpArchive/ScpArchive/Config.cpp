#include "Config.hpp"

#include <ctime>

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
		return "scp.karagory.com";
	}
	
	std::string getDatabaseHost(){
		return "scp-database";
	}
	
	std::string getDatabaseUser(){
		return "scparchive";
	}
	
	std::string getDatabasePassword(){
		return "password";
	}
	
	std::string getScraperFolder(){
		return "/scp_conversion_project/batchArchive/";
	}

	std::string getPDFFolder(){
		return "/scp_conversion_project/webRoot/__pdfs/";
	}
	
	std::string getWebRootStaticFolder(){
		return "/scp_conversion_project/webRoot/__static/";
	}
}




TimeStamp getCurrentTime(){
	return static_cast<TimeStamp>(std::time(nullptr));
}