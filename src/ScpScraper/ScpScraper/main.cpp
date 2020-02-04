
#include "Scraper/Scraper.hpp"

#include <iostream>
#include <boost/filesystem.hpp>

int main(int argc, char** argv){
	
	std::string dataFolder = "/home/daniel/File Collections/scpArchive/batchArchive/";
	std::string batchDataFile = dataFolder + "batchData.json";
	std::string batchesFolder = dataFolder + "batches/";
	
	if(argc == 2 && std::string(argv[1]) == "--makeInitialBatch"){
		std::string batchId = Scraper::generateInitialBatch(batchesFolder, batchDataFile);
		Scraper::downloadBatchData(batchesFolder + batchId + "/");
	}
	else if(argc == 2 && std::string(argv[1]) == "--makeDiffBatch"){
		std::string batchId = Scraper::generateDiffBatch(batchesFolder, batchDataFile);
		if(batchId == ""){
			std::cout << "Aborting new batch.\n";
		}
		else{
			Scraper::downloadBatchData(batchesFolder + batchId + "/");
			Scraper::checkBatchDownloads(batchesFolder + batchId + "/");
		}
	}
	else if(argc == 3 && std::string(argv[1]) == "--checkBatchDownloads"){
		Scraper::checkBatchDownloads(argv[2]);
	}
	else{
		std::cout << "Invalid/No Options Specified\n";
	}
	
	return 0;
}
