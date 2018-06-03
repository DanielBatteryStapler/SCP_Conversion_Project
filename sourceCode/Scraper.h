#pragma once

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <curl/curl.h>

#include <nlohmann/json.hpp>

#include <sstream>
#include <fstream>
#include <exception>
#include <string>

#include "Helpers.h"

struct ParserConvertData;

class Scraper{
public:
	Scraper() = delete;
	virtual ~Scraper();
	
	static std::vector<std::string> getPageList();
	
	static void getRawPageData(std::string page, std::string& pageId, std::string& title);
	static void getRawPageSource(std::string pageId, std::string& pageSource);
	static void downloadPageSource(std::string page);
	static void convertPageSourceToHtml(std::string page, std::string output, bool printFriendly, bool downloadImages);
	static void convertPageHtmlToPdf(std::string page, std::string input, std::string output);
	
	static void downloadImage(std::string image, ParserConvertData& convertData);
	/*
	static std::string getLatestRevisionId(std::string pageId);
	static std::string getRawRevisionSource(std::string revisionId);
	*/
};

