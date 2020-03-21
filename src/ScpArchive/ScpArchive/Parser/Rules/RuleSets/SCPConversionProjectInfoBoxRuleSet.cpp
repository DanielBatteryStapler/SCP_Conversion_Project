#include "SCPConversionProjectInfoBoxRuleSet.hpp"

#include "../../../Database/Database.hpp"
#include "../../../Database/Batch.hpp"

namespace Parser{
	nlohmann::json printNodeSCPConversionProjectInfoBox(const NodeVariant& nod){
		const SCPConversionProjectInfoBox& infoBox = std::get<SCPConversionProjectInfoBox>(nod);
		nlohmann::json out;
		out["lastUpdate"] = infoBox.lastUpdate;
		out["pageCount"] = infoBox.pageCount;
		out["revisionCount"] = infoBox.revisionCount;
		out["threadCount"] = infoBox.threadCount;
		out["postCount"] = infoBox.postCount;
		return out;
	}
	
    void handleSCPConversionProjectInfoBox(TreeContext& context, const Token& token){
        auto db = context.parameters.database;
        if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"SCPConversionProjectInfoBox Failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
        SCPConversionProjectInfoBox infoBox;
        {
			std::string scraperFolder = Config::getScraperFolder();
			infoBox.lastUpdate = Importer::lastImportedBatchTimeStamp(scraperFolder + "batches/", scraperFolder + "batchData.json");
        }
        infoBox.pageCount = db->getNumberOfPages();
        infoBox.revisionCount = db->getNumberOfPageRevisions();
        infoBox.threadCount = db->getNumberOfForumThreads();
        infoBox.postCount = db->getNumberOfForumPosts();
        addAsDiv(context, Node{infoBox});
    }
	
	void toHtmlNodeSCPConversionProjectInfoBox(const HtmlContext& con, const Node& nod){
		const SCPConversionProjectInfoBox& infoBox = std::get<SCPConversionProjectInfoBox>(nod.node);
		con.out << "<table><tbody>"_AM
		<< "<tr><td>Last Update</td><td>"_AM << formatTimeStamp(infoBox.lastUpdate) << "</td></tr>"_AM
		<< "<tr><td>Page Count</td><td>"_AM << std::to_string(infoBox.pageCount) << "</td></tr>"_AM
		<< "<tr><td>Revision Count</td><td>"_AM << std::to_string(infoBox.revisionCount) << "</td></tr>"_AM
		<< "<tr><td>Thread Count</td><td>"_AM << std::to_string(infoBox.threadCount) << "</td></tr>"_AM
		<< "<tr><td>Post Count</td><td>"_AM << std::to_string(infoBox.postCount) << "</td></tr>"_AM
		<< "</tbody></table>"_AM;
	}
}