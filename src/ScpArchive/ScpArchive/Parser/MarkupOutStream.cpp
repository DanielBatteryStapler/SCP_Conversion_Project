#include "MarkupOutStream.hpp"

#include <sstream>

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

MarkupOutStream::MarkupOutStream(std::ostream& outputStream):
	outStream(outputStream){
	
}

MarkupOutStream& MarkupOutStream::operator<<(std::string str){
	outStream << escapeMarkup(str);
	return *this;
}

MarkupOutStream& MarkupOutStream::operator<<(const MarkupOutStream::MarkupOutString& str){
	outStream << str.buffer;
	return *this;
}

MarkupOutStream& MarkupOutStream::operator<<(MarkupOutStream::MarkupOutString&& str){
	outStream << str.buffer;
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






