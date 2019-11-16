#include "UntestedUtils.hpp"

#include "../Database/Database.hpp"
#include "../Parser/Parser.hpp"
#include "../Config.hpp"

#include "../Database/Json.hpp"

#include <fstream>

namespace UntestedUtils{
	namespace{
		
		struct TimelineUpdate{
			enum class Type{Creation, Update};
			Type type;
			std::string name;
			std::string title;
			Database::TimeStamp timeStamp;
			std::vector<std::string> linkedNames;
		};
	}
	
	void exportNodeDiagramData(){
		std::unique_ptr<Database> database = Database::connectToMongoDatabase(Config::getProductionDatabaseName());
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
			for(Database::ID revisionId : revisionList){
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
			output.push_back(uj);
		}
		std::ofstream("nodeDiagramDataExport.json") << output.dump(4);
	}
}
