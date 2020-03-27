#include "UntestedUtils.hpp"

#include "../Database/Database.hpp"
#include "../Parser/Parser.hpp"
#include "../Parser/HTMLConverter.hpp"
#include "../Config.hpp"

#include "../Database/Json.hpp"

#include <fstream>
#include <chrono>

namespace UntestedUtils{
	namespace{
		
		struct TimelineUpdate{
			enum class Type{Creation, Update};
			Type type;
			std::string name;
			int64_t revisionIndex;
			std::string title;
			TimeStamp timeStamp;
			std::vector<std::string> linkedNames;
		};
	}
	
	void exportNodeDiagramData(){
		std::unique_ptr<Database> database = Database::connectToDatabase(Config::getProductionDatabaseName());
		std::vector<Database::ID> pageList = database->getPageList();
		
		std::vector<TimelineUpdate> timeline;
		
		int num = 0;
		for(Database::ID pageId : pageList){
			std::string name = database->getPageName(pageId);
			
			num++;
			std::cout << num << "/" << pageList.size() << " " << (num * 100.0 / pageList.size()) << "%: " << name << "...\n";
			
			std::vector<Database::ID> revisionList = database->getPageRevisions(pageId);
			if(revisionList.size() == 0){
				std::cout << "Error on page " << name << "\n";
			}
			bool creation = true;
			int64_t revisionIndex = 0;
			for(Database::ID revisionId : revisionList){
				std::cout << "    " << revisionIndex << "/" << revisionList.size() << "\r" << std::flush;

				TimelineUpdate update;
				if(creation){
					update.type = TimelineUpdate::Type::Creation;
					creation = false;
				}
				else{
					update.type = TimelineUpdate::Type::Update;
				}
				update.name = name;
				
				Database::PageRevision pageRevision = database->getPageRevision(revisionId);
				update.title = pageRevision.title;
				update.timeStamp = pageRevision.timeStamp;
				update.linkedNames = Parser::getPageLinks(pageRevision.sourceCode);
				update.revisionIndex = revisionIndex;
				revisionIndex++;
				
				timeline.push_back(update);
			}
		}
		
		std::sort(timeline.begin(), timeline.end(), [](const TimelineUpdate& a, const TimelineUpdate& b){return a.timeStamp < b.timeStamp;});
		nlohmann::json output = nlohmann::json::array();
		for(const TimelineUpdate& update : timeline){
			nlohmann::json uj;
			uj["type"] = (update.type == TimelineUpdate::Type::Creation)?"creation":"update";
			uj["name"] = update.name;
			uj["title"] = update.title;
			uj["timeStamp"] = update.timeStamp;
			uj["linkedNames"] = update.linkedNames;
			uj["revisionIndex"] = update.revisionIndex;
			output.push_back(uj);
		}
		try{
			std::ofstream("/home/daniel/File Collections/ScpNodeRender/nodeDiagramDataExportFinal.json") << output.dump(4);
		}
		catch(std::exception& e){
			while(true){
				std::cout << "Error Exporting!\nType out new directory:";
				std::string file;
				std::getline(std::cin, file);
				try{
					std::ofstream(file) << output.dump(4);
					break;
				}
				catch(std::exception& e){
					
				}
			}
		}
	}
	
	void benchmarkPageLoad(std::string pageName){
		std::unique_ptr<Database> database = Database::connectToDatabase(Config::getProductionDatabaseName());
		std::optional<Database::ID> pageId = database->getPageId(pageName);
		Database::PageRevision revision = database->getLatestPageRevision(pageId.value());
		
		std::stringstream outputString;
		MarkupOutStream outStream{&outputString};
		
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
		const auto getTime = [](){return std::chrono::high_resolution_clock::now();};
		const auto timeString = [](TimePoint before, TimePoint after)->std::string{return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count() / 1000000000.0);};
		
		TimePoint start = getTime();
		Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode);
		TimePoint endToken = getTime();
		Parser::PageTree pageTree = Parser::makeTreeFromTokenedPage(pageTokens);
		TimePoint endTree = getTime();
		Parser::convertPageTreeToHtml(outStream, pageTree);
		TimePoint endHtml = getTime();
		
		
		std::cout << "Token Duration: " << timeString(start, endToken) << "\n";
		std::cout << "Tree Duration: " << timeString(endToken, endTree) << "\n";
		std::cout << "HTML Duration: " << timeString(endTree, endHtml) << "\n";
	}
}

















