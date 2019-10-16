
#include "Scraper/Scraper.hpp"

#include <iostream>
#include <boost/filesystem.hpp>

int main(int argc, char** argv){
	
	std::string dataFolder = "/home/daniel/File Collections/scpArchive/fullArchive/";
	std::string pageList = dataFolder + "pageList.json";
	std::string pagesFolder = dataFolder + "pages/";
	std::string threadList = dataFolder + "threadList.json";
	std::string forumCategories = dataFolder + "forumCategories.json";
	std::string threadsFolder = dataFolder + "threads/";
	
	if(argc == 2 && std::string(argv[1]) == "--runTest"){
		/*
		std::string dataFolder = "/home/daniel/File Collections/scpArchive/";
		std::string pagesFolder = dataFolder + "pages/";
		std::string threadsFolder = dataFolder + "threads/";
		
		saveJsonToFile(dataFolder + "threadList.json", Scraper::getThreadListForAllCategories(loadJsonFromFile(dataFolder + "forumCategories.json")));
		Scraper::downloadFullPageArchive(pagesFolder, "scp-006");
		Scraper::downloadFullThreadArchive(threadsFolder, "76692");
		Scraper::downloadThreadList(threadsFolder, dataFolder + "testThreads.json");
		*/
		Scraper::downloadFullPageArchive(pagesFolder, "cephalopodstevenson-artwork");
	}
	else if(argc == 2 && std::string(argv[1]) == "--archive"){
		
		//boost::filesystem::remove_all(pagesFolder);
		//boost::filesystem::create_directory(pagesFolder);
		//boost::filesystem::remove_all(threadsFolder);
		//boost::filesystem::create_directory(threadsFolder);
		
		//saveJsonToFile(pageList, Scraper::getFullPageList());
		//saveJsonToFile(forumCategories, Scraper::getForumCategories());
		//saveJsonToFile(threadList, Scraper::getThreadListForAllCategories(loadJsonFromFile(forumCategories)));
		
		Scraper::downloadPageList(pagesFolder, pageList);
		//Scraper::downloadThreadList(threadsFolder, threadList);
	}
	else{
		std::cout << "Invalid/No Options Specified\n";
	}
}