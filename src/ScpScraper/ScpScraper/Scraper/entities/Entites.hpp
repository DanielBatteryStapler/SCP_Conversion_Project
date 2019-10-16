#ifndef ENTITIES_HPP
#define ENTITIES_HPP

extern "C"{
	#include "entities.h"
}

static std::string decodeHtmlEntities(std::string input){
	char* outputBuffer = new char[input.size() + 1];
	std::size_t outputSize = decode_html_entities_utf8(outputBuffer, input.c_str());
	std::string output{outputBuffer, outputSize};
	return output;
}

#endif // ENTITIES_HPP