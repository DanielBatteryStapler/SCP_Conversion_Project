#ifndef JSON_HPP
#define JSON_HPP

#include <nlohmann/json.hpp>

namespace Json{
	nlohmann::json loadJsonFromFile(std::string fileName);
}

#endif // JSON_HPP
