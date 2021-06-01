#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "nlohmann/json.hpp"

namespace Database{
	nlohmann::json getPageTree(std::string name);
	
	nlohmann::json getPage(std::string name);
	void loadImagesIntoPageTree(nlohmann::json& pageTree);
}

#endif // DATABASE_HPP
