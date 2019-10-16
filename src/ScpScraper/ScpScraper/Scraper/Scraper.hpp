#ifndef SCAPER_HPP
#define SCAPER_HPP

#include <string>
#include <map>

#include <nlohmann/json.hpp>

#include "Helpers.hpp"

namespace Scraper{
	nlohmann::json performAjaxRequest(std::string moduleName, std::map<std::string, std::string> parameters);
	
	std::vector<std::string> getFullPageList();
	
	void downloadPageList(std::string pagesFolder, std::string pageListFile);
	void downloadFullPageArchive(std::string pagesFolder, std::string pageName);
	
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
	
	nlohmann::json getForumCategories();
	std::vector<std::string> getThreadListForForumCategory(std::string categoryId);
	std::vector<std::string> getThreadListForAllCategories(nlohmann::json forumCategories);
	
	void downloadThreadList(std::string threadsFolder, std::string threadListFile);
	void downloadFullThreadArchive(std::string threadsFolder, std::string threadId);
	nlohmann::json getThreadPostReplies(const nlohmann::json& divTree, int offset);
	nlohmann::json getThreadPost(const nlohmann::json& divTree);
}

#endif // SCAPER_HPP
