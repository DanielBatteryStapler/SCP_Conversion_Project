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
	
	std::string getDatabaseUser();
	std::string getDatabasePassword();
	
	std::string getScraperFolder();
}

#endif // CONFIG_HPP
