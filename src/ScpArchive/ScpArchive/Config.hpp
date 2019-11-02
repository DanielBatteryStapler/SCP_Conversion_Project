#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config{
	std::string getTestingDatabaseName();
	
	unsigned int getNumberOfThreadsForGateway();
	std::string	getProductionDatabaseName();
	std::string getFastCGISocket();
	std::string getWebsiteDomainName();
}

#endif // CONFIG_HPP
