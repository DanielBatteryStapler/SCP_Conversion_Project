#ifndef SCAPER_HPP
#define SCAPER_HPP

#include <string>
#include <map>

#include <nlohmann/json.hpp>

#include "Helpers.hpp"

namespace Scraper{
	std::string generateInitialBatch(std::string batchesFolder, std::string batchDataFile);
	std::string generateDiffBatch(std::string batchesFolder, std::string batchDataFile);
	
	void downloadBatchData(std::string batchFolder);
	void checkBatchDownloads(std::string batchesFolder, std::string batchDataFile, std::string batchFolder);
	void checkAllBatchDownloads(std::string batchesFolder, std::string batchDataFile);
	void checkLastBatchDownload(std::string batchesFolder, std::string batchDataFile);
	
	std::vector<std::string> getFullPageList();
	std::vector<std::string> getUpdatedPageList(std::int64_t startTime);
	
	void downloadPageList(std::string pagesFolder, std::vector<std::string> pageList);
	void downloadFullPageArchive(std::string pagesFolder, std::string pageName);
	
	bool pageExists(std::string pageName);
	std::string getPageId(std::string pageName);
	std::string getPageParent(std::string pageName);
	std::string getPageDiscussionId(std::string pageName);
	nlohmann::json getPageRevisions(std::string pageId);
	nlohmann::json getRevisionData(std::string revisionId);
	std::string getRevisionSource(std::string revisionId);
	nlohmann::json getPageFiles(std::string pageId);
	nlohmann::json getFileData(std::string fileId);
	nlohmann::json getPageTags(std::string pageName);
	nlohmann::json getPageVotes(std::string pageId);
	
	void downloadImage(std::string fileName, std::string fileUrl);
	
	nlohmann::json getForumGroups();
	std::vector<std::string> getThreadListForForumCategory(std::string categoryId);
	std::vector<std::string> getThreadListForAllCategories(nlohmann::json forumGroups);
	std::vector<std::string> getUpdatedThreadList(std::int64_t startTime);
	
	bool threadExists(std::string threadId);
	void downloadThreadList(std::string threadsFolder, std::vector<std::string> threadList);
	void downloadFullThreadArchive(std::string threadsFolder, std::string threadId);
	nlohmann::json getThreadPostReplies(const nlohmann::json& divTree, int offset);
	nlohmann::json getThreadPost(const nlohmann::json& divTree);
	
	std::vector<std::string> getAuthorList(std::string pagesFolder, std::vector<std::string> pageList, std::string threadsFolder, std::vector<std::string> threadList);
	void downloadAuthorList(std::string authorsFile, std::vector<std::string> authorList);
	nlohmann::json getAuthorData(std::string authorId);
}

#endif // SCAPER_HPP
