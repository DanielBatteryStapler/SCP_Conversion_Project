#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config{
	std::string getTestingDatabaseName();
    
    std::string getImportMapFileName();
    
	unsigned int getNumberOfThreadsForGateway();
	std::string	getProductionDatabaseName();
	std::string getFastCGISocket();
	std::string getWebsiteDomainName();
	
	std::string getDatabaseHost();
	std::string getDatabaseUser();
	std::string getDatabasePassword();
	
	std::string getScraperFolder();
	std::string getPDFFolder();
	std::string getWebRootStaticFolder();
}

using TimeStamp = std::int64_t;
TimeStamp getCurrentTime();

#endif // CONFIG_HPP
