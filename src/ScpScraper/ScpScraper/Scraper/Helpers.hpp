#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

nlohmann::json loadJsonFromFile(std::string fileName);
void saveJsonToFile(std::string fileName, nlohmann::json data);

std::size_t getData(const std::string& data, std::string start, std::string end, std::size_t findPos, std::string& output);

std::string& trimLeft(std::string& s);
std::string& trimRight(std::string& s);
std::string& trimString(std::string& s);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

std::string percentDecode(const std::string& SRC);
std::string percentEncode(const std::string &value);

#endif // HELPERS_HPP
