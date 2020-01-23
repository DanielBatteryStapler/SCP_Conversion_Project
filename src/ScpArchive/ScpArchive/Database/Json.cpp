#include "Json.hpp"

#include <fstream>

namespace Json{
	nlohmann::json loadJsonFromFile(std::string fileName){
		std::ifstream file(fileName);
		if(!file){
			throw std::runtime_error("Attempted to JSON from file \"" + fileName + "\", but could not read from that file");
		}
		
		std::vector<std::uint8_t> fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		std::string fileContentsStr(fileContents.begin(), fileContents.end());
		return nlohmann::json::parse(fileContentsStr);
	}
}


















