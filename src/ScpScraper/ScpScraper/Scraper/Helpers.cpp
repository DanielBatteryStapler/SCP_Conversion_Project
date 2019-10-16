#include "Helpers.hpp"

#include <fstream>

nlohmann::json loadJsonFromFile(std::string fileName){
	std::ifstream file(fileName);
	std::vector<uint8_t> fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::string fileContentsStr(fileContents.begin(), fileContents.end());
	nlohmann::json data = nlohmann::json::parse(fileContentsStr);
	return data;
}

void saveJsonToFile(std::string fileName, nlohmann::json data){
	std::ofstream(fileName) << data.dump(4);
}

std::size_t getData(const std::string& data, std::string start, std::string end, std::size_t findPos, std::string& output){
	std::size_t startPos = data.find(start, findPos);
	if(startPos == std::string::npos){
		return std::string::npos;
	}
	startPos += start.size();
	std::size_t endPos = data.find(end, startPos);
	if(endPos == std::string::npos){
		output = data.substr(startPos, output.size() - 1 - startPos);
		return std::string::npos;
	}
	output = data.substr(startPos, endPos - startPos);
	return endPos + end.size();
}

std::string& trimLeft(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}
// trim from end
std::string& trimRight(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}
// trim from both ends
std::string& trimString(std::string& s) {
	return trimLeft(trimRight(s));
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    std::size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::string percentDecode(const std::string& SRC){//works with utf8
	std::string ret;
    char ch;
    unsigned int i, ii;
    for (i = 0; i < SRC.length(); i++) {
        if (int(SRC[i]) == 37) {
            sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return ret;
}

std::string percentEncode(const std::string &value) {//works with utf8
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

