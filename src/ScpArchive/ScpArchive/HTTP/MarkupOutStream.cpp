#include "MarkupOutStream.hpp"

#include <sstream>
#include <iomanip>

#include <cgicc/CgiUtils.h>

std::string urlEncode(const std::string& text){
	std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : text) {
        //Keep alphanumeric and other accepted characters intact
        if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~'){
            escaped << c;
            continue;
        }

        //special case for spaces
        if(c == ' '){
			escaped << '+';
			continue;
        }
        
        //Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        escaped << std::nouppercase;
    }

    return escaped.str();
}

std::string urlDecode(const std::string& text){
	//for now we're just going to rely on cgicc's implementation for this
	return cgicc::form_urldecode(text);
}

namespace{
	std::string escapeMarkup(const std::string& str){
		std::stringstream output;
		for(char c : str){
			switch(c){
			case '<':
				output << "&lt;";
				break;
			case '>':
				output << "&gt;";
				break;
			case '&':
				output << "&amp;";
				break;
			case ';':
				output << "&#59;";
				break;
			case '"':
				output << "&quot;";
				break;
			case '\'':
				output << "&#39;";
				break;
			default:
				output << c;
				break;
			}
		}
		return output.str();
	}
}

MarkupOutStream::MarkupOutStream():
	outStream(nullptr){
	
}

MarkupOutStream::MarkupOutStream(std::ostream* outputStream):
	outStream(outputStream){
	
}

MarkupOutStream& MarkupOutStream::operator<<(char c){
	*outStream << escapeMarkup(std::string(1, c));
	return *this;
}

MarkupOutStream& MarkupOutStream::operator<<(std::string str){
	*outStream << escapeMarkup(str);
	return *this;
}

MarkupOutStream& MarkupOutStream::operator<<(const MarkupOutStream::MarkupOutString& str){
	*outStream << str.buffer;
	return *this;
}

MarkupOutStream& MarkupOutStream::operator<<(MarkupOutStream::MarkupOutString&& str){
	*outStream << str.buffer;
	return *this;
}

MarkupOutStream::MarkupOutString::MarkupOutString(std::string str):
	buffer(str){
	
}

MarkupOutStream::MarkupOutString allowMarkup(std::string str){
	return MarkupOutStream::MarkupOutString{str};
}

MarkupOutStream::MarkupOutString operator""_AM(const char* str, long unsigned int length){
	return MarkupOutStream::MarkupOutString{std::string{str, length}};
}






