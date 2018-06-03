#pragma once

#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>

std::size_t getData(std::string& data, std::string start, std::string end, std::size_t findPos, std::string& output);

std::string& trimLeft(std::string& s);
std::string& trimRight(std::string& s);
std::string& trimString(std::string& s);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

std::string percentDecode(const std::string& SRC);
std::string percentEncode(const std::string &value);