#ifndef JSON_HPP
#define JSON_HPP

#include <nlohmann/json.hpp>

namespace Json{
	nlohmann::json loadJsonFromFile(std::string fileName);
	void saveJsonToFile(std::string fileName, nlohmann::json data);
}

#endif // JSON_HPP
