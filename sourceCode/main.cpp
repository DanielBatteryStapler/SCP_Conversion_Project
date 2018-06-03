#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <nlohmann/json.hpp>

#include <sstream>
#include <fstream>
#include <exception>
#include <string>
#include <thread>
#include <chrono>

#include "Scraper.h"
#include "Parser.h"

std::vector<std::string> loadPageList(std::string listFile){
	std::vector<std::string> output;
	std::ifstream file(listFile);
	std::string page;
	while(std::getline(file, page, '\n')){
		trimString(page);
		if(page == ""){
			continue;
		}
		if(page.find("//") == std::string::npos){
			output.push_back(page);
		}
		else{//if there is a comment in the listfile(denoted by "//"), remove it
			output.push_back(page.substr(0, page.find("//")));
			trimString(output.back());
			if(output.back() == ""){
				output.pop_back();
			}
		}
	}
	return output;
}

int main(int argc, char** argv){
	
	std::string action;
	std::string argument;
	
	if(argc == 2 || argc == 3){
		std::string c = argv[1];
		if(c == "--help" || c == "-h" || c == "help"){
			action = "help";
		}
		else if(c == "downloadPageList"){
			action = "downloadPageList";
		}
		else if(c == "downloadPage"){
			action = "downloadPage";
		}
		else if(c == "downloadAllPages"){
			action = "downloadAllPages";
		}
		else if(c == "convertToHtml"){
			action = "convertToHtml";
		}
		else if(c == "convertAllToHtml"){
			action = "convertAllToHtml";
		}
		else if(c == "convertToPdf"){
			action = "convertToPdf";
		}
		else if(c == "convertAllToPdf"){
			action = "convertAllToPdf";
		}
		else if(c == "performFullToolChain"){
			action = "performFullToolChain";
		}
		else{
			throw std::runtime_error("Invalid Action \"" + c + "\"");
		}
		
		if(action != "help"){
			if(argc != 3){
				throw std::runtime_error("Invalid Number of Arguments");
			}
			argument = argv[2];
		}
	}
	else{
		if(argc > 3){
			throw std::runtime_error("Invalid Number of Arguments");
		}
	}
	
	if(action == "help" || action == ""){
		std::cout <<
		"\n"
		"   >>Make sure to run this program while the current directory is the ScpScraper data folder!<<\n"
		"\n"
		"Usage:\n"
		"ScpScraper help                            display this help page\n"
		"ScpScraper performFullToolChain [PAGE]     perform the full downloadPage->convertToHtml->convertToPdf toolchain on a single PAGE\n"
		"ScpScraper downloadPageList [LISTFILE]     download a full list of pages on the SCP Wiki and save them as LISTFILE\n"
		"ScpScraper downloadPage [PAGE]             download the WikiDotSource file for PAGE\n"
		"ScpScraper downloadAllPages [LISTFILE]     download all of the pages in the LISTFILE file\n"
		"ScpScraper convertToHtml [PAGE]            convert the downloaded PAGE WikiDotSource into the WebsiteHtml and PdfHtml and download any attached images\n"
		"ScpScraper convertAllToHtml [LISTFILE]     perform \"convertToHtml\" on all pages in the LISTFILE\n"
		"ScpScraper convertToPdf [PAGE]             convert the already converted PAGE PdfHtml and attached images into a single PDF file\n"
		"ScpScraper convertAllToPdf [LISTFILE]      perform \"convertToPdf\" on all pages in the LISTFILE\n";
		
		return 0;
	}
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	std::vector<std::string> pagesWithErrors;//save a list of pages with errors so they can be shown later
	
	if(action == "downloadPageList"){
		std::ofstream file(argument);
		std::vector<std::string> output = Scraper::getPageList();
		for(auto i = output.begin(); i != output.end(); i++){
			if(i->find(":") == std::string::npos){//pages with colons in them are special and I don't know how to deal with them yet, so we're just gonna ignore them
				file << *i << "\n";
			}
		}
		file.close();
	}
	else if(action == "downloadPage"){
		Scraper::downloadPageSource(argument);
	}
	else if(action == "downloadAllPages"){
		std::vector<std::string> pageList = loadPageList(argument);
		std::cout << "Page List Size:" << pageList.size() << "\n";
		int num = 0;
		for(auto i = pageList.begin(); i != pageList.end(); i++){
			std::cout << num << ". Downloading \"" << *i << "\"...\n";
			try{
				Scraper::downloadPageSource(*i);
			}
			catch(std::exception& e){
				std::cout << "==========CAUGHT EXCEPTION: " << e.what() << "\n";
				pagesWithErrors.push_back(*i);
			}
			num++;
		}
	}
	else if(action == "convertToHtml"){
		Scraper::convertPageSourceToHtml(argument, argument + ".websiteVersion.html", false, false);
		Scraper::convertPageSourceToHtml(argument, argument + ".pdfVersion.html", true, true);
	}
	else if(action == "convertAllToHtml"){
		std::vector<std::string> pageList = loadPageList(argument);
		std::cout << "Page List Size:" << pageList.size() << "\n";
		int num = 0;
		for(auto i = pageList.begin(); i != pageList.end(); i++){
			std::cout << num << ". Converting \"" << *i << "\" to HTML(Website Version)...\n";
			try{
				Scraper::convertPageSourceToHtml(*i, *i + ".websiteVersion.html", false, false);
			}
			catch(std::exception& e){
				std::cout << "==========CAUGHT EXCEPTION: " << e.what() << "\n";
				pagesWithErrors.push_back(*i);
			}
			num++;
		}
		num = 0;
		for(auto i = pageList.begin(); i != pageList.end(); i++){
			std::cout << num << ". Converting \"" << *i << "\" to HTML(PDF Version)...\n";
			try{
				Scraper::convertPageSourceToHtml(*i, *i + ".pdfVersion.html", true, true);
			}
			catch(std::exception& e){
				std::cout << "==========CAUGHT EXCEPTION: " << e.what() << "\n";
				//pagesWithErrors.push_back(*i);//if there was an error, it would have already been caught in the first conversion
			}
			num++;
		}
	}
	else if(action == "convertToPdf"){
		Scraper::convertPageHtmlToPdf(argument, argument + ".pdfVersion.html", argument + ".pdf");
	}
	else if(action == "convertAllToPdf"){
		std::vector<std::string> pageList = loadPageList(argument);
		std::cout << "Page List Size:" << pageList.size() << "\n";
		int num = 0;
		for(auto i = pageList.begin(); i != pageList.end(); i++){
			std::cout << num << ". Converting \"" << *i << "\" to PDF...\n";
			try{
				Scraper::convertPageHtmlToPdf(*i, *i + ".pdfVersion.html", *i + ".pdf");
			}
			catch(std::exception& e){
				std::cout << "==========CAUGHT EXCEPTION: " << e.what() << "\n";
				pagesWithErrors.push_back(*i);
			}
			num++;
		}
	}
	else if(action == "performFullToolChain"){
		Scraper::downloadPageSource(argument);
		Scraper::convertPageSourceToHtml(argument, argument + ".websiteVersion.html", false, false);
		Scraper::convertPageSourceToHtml(argument, argument + ".pdfVersion.html", true, true);
		Scraper::convertPageHtmlToPdf(argument, argument + ".pdfVersion.html", argument + ".pdf");
	}
	
	if(pagesWithErrors.size() > 0){
		std::cout << "\n\n				PAGES WITH ERRORS:\n";
		for(auto i = pagesWithErrors.begin(); i != pagesWithErrors.end(); i++){
			std::cout << *i << "\n";
		}
	}
	
	curl_global_cleanup();
	
	return 0;
}






