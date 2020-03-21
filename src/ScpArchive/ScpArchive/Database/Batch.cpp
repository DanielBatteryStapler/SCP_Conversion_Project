#include "Batch.hpp"

#include "Json.hpp"
#include "../Config.hpp"

#include <iostream>
#include <ctime>
#include <iomanip>

namespace Importer{
	namespace{
		struct TimelineEntry{
			std::string batchId;
			TimeStamp timeStamp;
			enum Status{Applied, Available, Error};
			Status status;
			
			bool operator<(TimelineEntry a)const{
				return timeStamp < a.timeStamp;
			}
		};
		
		std::vector<TimelineEntry> getTimeline(std::string batchesFolder, std::string batchDataFile){
			nlohmann::json batchData = Json::loadJsonFromFile(batchDataFile);
			if(batchData["batchErrors"].size() > 0){
				throw std::runtime_error("There are Unhandled Batch Errors, Aborting.");
			}
			
			const auto getTimeStamp = [&](std::string batchId)->TimeStamp{
				nlohmann::json batch = Json::loadJsonFromFile(batchesFolder + batchId + "/batch.json");
				return batch["timeStamp"].get<TimeStamp>();
			};
			
			
			std::vector<TimelineEntry> timeline;
			
			std::vector<std::string> available = batchData["availableBatches"];
			std::vector<std::string> applied = batchData["appliedBatches"];
			
			for(const auto batch : available){
				if(std::find(applied.begin(), applied.end(), batch) != applied.end()){
					timeline.push_back(TimelineEntry{batch, getTimeStamp(batch), TimelineEntry::Applied});
				}
				else{
					timeline.push_back(TimelineEntry{batch, getTimeStamp(batch), TimelineEntry::Available});
				}
			}
			
			for(const auto batch : applied){
				if(std::find(available.begin(), available.end(), batch) == available.end()){
					timeline.push_back(TimelineEntry{batch, getTimeStamp(batch), TimelineEntry::Error});
				}
			}
			
			std::sort(timeline.begin(), timeline.end());
			return timeline;
		}
		
		void printTimeline(const std::vector<TimelineEntry>& timeline){
			std::size_t textSize = 0;
			for(const auto entry : timeline){
				textSize = std::max(entry.batchId.size(), textSize);
			}
			
			std::cout << "Batch Timeline:\n";
			for(std::size_t i = 0; i < timeline.size(); i++){
				const auto& entry = timeline[i];
				std::cout << "    " << entry.batchId
				<< std::string(textSize - entry.batchId.size(), ' ')
				<< " [";
				switch(entry.status){
					case TimelineEntry::Applied:
						std::cout << "OLD";
						break;
					case TimelineEntry::Available:
						std::cout << "NEW";
						break;
					case TimelineEntry::Error:
						std::cout << "ERR";
						break;
				}
				std::cout << "] @ " << std::put_time(std::gmtime(&entry.timeStamp), "%a %b %d %H:%M:%S %Y") << "\n";
				if(i < timeline.size() - 1){
					std::cout << "          |\n";
					std::cout << "          V\n";
				}
			}
			std::cout << "\n\n";
		}
		
		bool timelineHasError(const std::vector<TimelineEntry>& timeline){
			bool error = false;		
			{
				bool availableFound = false;
				for(const auto entry : timeline){
					switch(entry.status){
						case TimelineEntry::Applied:
							if(availableFound){
								error = true;
							}
							break;
						case TimelineEntry::Available:
							availableFound = true;
							break;
						case TimelineEntry::Error:
							error = true;
							break;
					}
					
				}
			}
			return error;
		}
		
		std::string getNextImport(const std::vector<TimelineEntry>& timeline){
			std::string nextImport;
			for(const auto entry : timeline){
				if(entry.status == TimelineEntry::Available){
					nextImport = entry.batchId;
					break;
				}
			}
			return nextImport;
		}
	}
	
	void handleBatches(std::string batchesFolder, std::string batchDataFile, bool automatic){
		std::vector<TimelineEntry> timeline = getTimeline(batchesFolder, batchDataFile);
		
		std::cout << "\n\n";
		printTimeline(timeline);
		
		if(timelineHasError(timeline)){
			throw std::runtime_error("There is an Error in the Batch Timeline, Aborting.");
		}
		
		std::string nextImport = getNextImport(timeline);
		
		std::cout << "Available Actions:\n"
		 << "  0. Exit\n"
		 << "  1. Automatic Import\n"
		 << "  2. Reset Timeline\n";
		if(nextImport != ""){
			std::cout << "  3. Import Batch \"" << nextImport << "\"\n";
		}
		int choice = -1;
		if(!automatic){
			while(true){
				std::cout << ">";
				std::string input;
				std::getline(std::cin, input);
				try{
					choice = std::stoi(input);
				}
				catch(std::invalid_argument& e){
					continue;
				}
				if(choice < 0 || choice > 3){
					continue;
				}
				if(nextImport == "" && choice == 3){
					continue;
				}
				break;
			}
		}
		else{
			automatic = 1;
			choice = 1;
		}
		
		try{
			switch(choice){
				case 0:
					{
						//do nothing
					}
					break;
				case 1:
					{
						while(nextImport != ""){
							{
								std::cout << "Importing Batch \"" << nextImport << "\"...\n";
								importBatch(batchesFolder + nextImport + "/");
								
								nlohmann::json batchData = Json::loadJsonFromFile(batchDataFile);
								batchData["appliedBatches"].push_back(nextImport);
								Json::saveJsonToFile(batchDataFile, batchData);
								std::cout << "Batch \"" << nextImport << "\" Complete.\n";
							}
							timeline = getTimeline(batchesFolder, batchDataFile);
							std::cout << "Timeline Status:\n";
							printTimeline(timeline);
							nextImport = getNextImport(timeline);
						}
						std::cout << "Timeline Complete.\n";
					}
					break;
				case 2:
					{
						nlohmann::json batchData = Json::loadJsonFromFile(batchDataFile);
						batchData["appliedBatches"].clear();
						Json::saveJsonToFile(batchDataFile, batchData);
					}
					break;
				case 3:
					{
						std::cout << "Importing Batch \"" << nextImport << "\"...\n";
						importBatch(batchesFolder + nextImport + "/");
						
						nlohmann::json batchData = Json::loadJsonFromFile(batchDataFile);
						batchData["appliedBatches"].push_back(nextImport);
						Json::saveJsonToFile(batchDataFile, batchData);
						std::cout << "Batch \"" << nextImport << "\" Complete.\n";
					}
					break;
			}
		}
		catch(std::exception& e){
			nlohmann::json batchData = Json::loadJsonFromFile(batchDataFile);
			nlohmann::json error;
			std::cout << "Exception Error: " << e.what() << "\n";
			error["error"] = e.what();
			error["batchId"] = nextImport;
			batchData["batchErrors"].push_back(error);
			Json::saveJsonToFile(batchDataFile, batchData);
			
			throw e;
		}
	}
	
	void importBatch(std::string batchFolder){
		nlohmann::json batch = Json::loadJsonFromFile(batchFolder + "batch.json");
		auto database = Database::connectToDatabase(Config::getProductionDatabaseName());
		ImportMap map(database.get());
		
		std::string type = batch["type"].get<std::string>();
		if(type == "initial"){
			database->cleanAndInitDatabase();
			
			importAuthors(database.get(), map, Json::loadJsonFromFile(batchFolder + "authors.json"));
			
			importForumGroups(database.get(), map, batch["forumGroups"]);
			importThreadsFromFolder(database.get(), map, batchFolder + "threads/", batch["threadList"]);
			
			importPagesFromFolder(database.get(), map, batchFolder + "pages/", batch["pageList"]);
			uploadPageFilesFromFolder(database.get(), map, batchFolder + "pages/", batch["pageList"]);
		}
		else if(type == "diff" || type == "user"){
			
			importAuthors(database.get(), map, Json::loadJsonFromFile(batchFolder + "authors.json"));
			
			importThreadsFromFolder(database.get(), map, batchFolder + "threads/", batch["threadList"]);
			
			importPagesFromFolder(database.get(), map, batchFolder + "pages/", batch["pageList"]);
			uploadPageFilesFromFolder(database.get(), map, batchFolder + "pages/", batch["pageList"]);
		}
		else{
			throw std::runtime_error("Batch has Invalid Type.");
		}
	}
	
	TimeStamp lastImportedBatchTimeStamp(std::string batchesFolder, std::string batchDataFile){
		std::vector<TimelineEntry> timeline = getTimeline(batchesFolder, batchDataFile);
		if(timelineHasError(timeline)){
			return 0;
		}
		TimeStamp newestAppliedBatchTimeStamp = 0;
		for(const TimelineEntry& entry : timeline){
			if(entry.status == TimelineEntry::Applied){
				newestAppliedBatchTimeStamp = entry.timeStamp;
			}
			else{
				break;
			}
		}
		return newestAppliedBatchTimeStamp;
	}
}
