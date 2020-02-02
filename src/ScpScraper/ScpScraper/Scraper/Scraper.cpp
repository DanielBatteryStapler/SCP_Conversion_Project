#include "Scraper.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <curl/curl.h>

#include <sstream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <stack>
#include <ctime>

#include <boost/filesystem.hpp>

#include "entities/Entites.hpp"

namespace Scraper{
	namespace{
		const std::string website = "http://www.scp-wiki.net/";
		const std::string userAgent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.89 Safari/537.36";
		const std::string token7 = "666656";
		const std::string noRedirect = "/noredirect/true";
		
		nlohmann::json performAjaxRequest(std::string moduleName, std::map<std::string, std::string> parameters){
			curlpp::Cleanup myCleanup;
			curlpp::Easy request;
			request.setOpt<curlpp::options::Url>(website + "ajax-module-connector.php");
			
			//request.setOpt<curlpp::options::Verbose>(true);
			
			std::list<std::string> headers;
			headers.push_back("X-Requested-With: XMLHttpRequest");
			headers.push_back(userAgent);
			headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
			headers.push_back("Accept: application/json");
			headers.push_back("Accept-Encoding: identity");
			headers.push_back("Accept-Language: en-US,en;q=0.9");
			headers.push_back("Cookie: wikidot_token7=" + percentEncode(token7));
			request.setOpt<curlpp::options::HttpHeader>(headers);
			
			std::string body = "moduleName=" + percentEncode(moduleName);
			
			parameters["wikidot_token7"] = token7;//just automatically set the token 7
			
			for(auto i = parameters.begin(); i != parameters.end(); i++){
				body += "&" + percentEncode(i->first) + "=" + percentEncode(i->second);
			}
			
			request.setOpt<curlpp::options::PostFields>(body);
			request.setOpt<curlpp::options::PostFieldSize>(body.length());
			
			std::ostringstream os;
			curlpp::options::WriteStream ws(&os);
			request.setOpt(ws);
			request.perform();
			std::string data = os.str();
			
			nlohmann::json page = nlohmann::json::parse(data);
			return page;
		}
	}
	
	std::string generateInitialBatch(std::string batchesFolder, std::string batchDataFile){
		std::int64_t timeStamp = static_cast<std::int64_t>(std::time(nullptr));
		std::string batchId = std::to_string(timeStamp) + "-initial";
		
		nlohmann::json pageList = getFullPageList();
		nlohmann::json forumGroups = getForumGroups();
		nlohmann::json threadList = getThreadListForAllCategories(forumGroups);
		
		nlohmann::json batch;
		batch["type"] = "initial";
		batch["timeStamp"] = timeStamp;
		batch["pageList"] = pageList;
		batch["forumGroups"] = forumGroups;
		batch["threadList"] = threadList;
		
		boost::filesystem::remove_all(batchesFolder);
		boost::filesystem::create_directory(batchesFolder);
		
		std::string batchFolder = batchesFolder + batchId + "/";
		boost::filesystem::create_directory(batchFolder);
		saveJsonToFile(batchFolder + "batch.json", batch);
		
		boost::filesystem::remove_all(batchDataFile);
		nlohmann::json batchData;
		batchData["availableBatches"] = nlohmann::json::array();
		batchData["availableBatches"].push_back(batchId);
		batchData["appliedBatches"] = nlohmann::json::array();
		batchData["batchErrors"] = nlohmann::json::array();
		saveJsonToFile(batchDataFile, batchData);
		
		return batchId;
	}
	
	std::string generateDiffBatch(std::string batchesFolder, std::string batchDataFile){
		nlohmann::json batchData = loadJsonFromFile(batchDataFile);
		if(batchData["batchErrors"].size() > 0){
            throw std::runtime_error("Cannot create Diff Batch, Timeline File has errors");
		}
		std::int64_t oldTime = 0;
		for(std::string lastBatch : batchData["availableBatches"]){
			oldTime = std::max(oldTime, loadJsonFromFile(batchesFolder + lastBatch + "/batch.json")["timeStamp"].get<std::int64_t>());
		}
		
		std::int64_t timeStamp = static_cast<std::int64_t>(std::time(nullptr));
		std::string batchId = std::to_string(timeStamp) + "-diff";
		
		nlohmann::json batch;
		batch["type"] = "diff";
		batch["timeStamp"] = timeStamp;
		batch["pageList"] = getUpdatedPageList(oldTime - 60);//we're just gonna be safe and actually start like a while before we need to
		batch["threadList"] = nlohmann::json::array();
		
		if(batch["pageList"].size() == 0 && batch["threadList"].size() == 0){
			std::cout << "No new content found.\n";
			return "";
		}
		
		std::string batchFolder = batchesFolder + batchId + "/";
		boost::filesystem::create_directory(batchFolder);
		saveJsonToFile(batchFolder + "batch.json", batch);
		
		batchData["availableBatches"].push_back(batchId);
		saveJsonToFile(batchDataFile, batchData);
		
		return batchId;
	}
	
	void downloadBatchData(std::string batchFolder){
		std::cout << "Downloading batch data...\n";
		nlohmann::json batch = loadJsonFromFile(batchFolder + "batch.json");
		
		{
			std::string pagesFolder = batchFolder + "pages/";
			boost::filesystem::remove_all(pagesFolder);
			boost::filesystem::create_directory(pagesFolder);
			std::vector<std::string> pageList = batch["pageList"];
			downloadPageList(pagesFolder, pageList);
		}
		{
			std::string threadsFolder = batchFolder + "threads/";
			boost::filesystem::remove_all(threadsFolder);
			boost::filesystem::create_directory(threadsFolder);
			std::vector<std::string> threadList = batch["threadList"];
			downloadThreadList(threadsFolder, threadList);
		}
	}
	
	void checkBatchDownloads(std::string batchFolder){
		std::cout << "Checking for download errors...\n";
		nlohmann::json batch = loadJsonFromFile(batchFolder + "batch.json");
		std::string pagesFolder = batchFolder + "pages/";
		
		std::vector<std::string> pageErrors;
		for(std::string page : batch["pageList"]){
			std::string pageFile = pagesFolder + page + "/data.json";
			if(!boost::filesystem::exists(pageFile) || boost::filesystem::is_directory(pageFile)){
				pageErrors.push_back(page);
			}
		}
		if(pageErrors.size() > 0){
			std::cout << "Errors:\n";
			for(std::string page : pageErrors){
				std::cout << "\t" << page << "\n";
			}
			std::cout << "Attempting to fix errors.";
			downloadPageList(pagesFolder, pageErrors);
		}
		else{
			std::cout << "No errors in Batch Download.\n";
		}
	}
	
	std::vector<std::string> getFullPageList(){
		
		//this is very ugly code but it works so it's all fine
		//I wish I could pretty it up by using the ajax, but I can't because the ajax doesn't show "fragment:" pages afaik
		
		std::vector<std::string> output;
		
		int pageNumber = 1;
		
		while(true){
			std::cout << "Current Page of Listing:" << pageNumber << "\n";
			curlpp::Cleanup myCleanup;
			curlpp::Easy request;
			request.setOpt<curlpp::options::Url>(website + "system:list-all-pages/p/" + std::to_string(pageNumber));
			
			std::list<std::string> headers;
			headers.push_back(userAgent);
			headers.push_back("Accept: test/html");
			headers.push_back("Accept-Encoding: identity");//gzip, deflate
			headers.push_back("Accept-Language: en-US,en;q=0.9");
			request.setOpt<curlpp::options::HttpHeader>(headers);
			
			std::ostringstream os;
			curlpp::options::WriteStream ws(&os);
			request.setOpt(ws);
			request.perform();
			std::string data = os.str();
			{
				std::string currentPage;
				getData(data, "<span class=\"current\">", "</span>", 0, currentPage);
				if(currentPage != std::to_string(pageNumber)){
					std::cout << "Current Listing Page is Empty; Reached End of Pages.\n";
					break;
				}
			}
			{
				std::string pageListData;
				getData(data, "<div class=\"list-pages-box\">", "<div class=\"pager\">", 0, pageListData);
				std::size_t start = 0;
				while(true){
					std::string temp;
					start = getData(pageListData, "<p><a href=\"/", "\"", start, temp);
					if(start == std::string::npos){
						break;
					}
					output.push_back(temp);
				}
			}
			pageNumber++;
		}
		
		return output;
	}
	
	std::vector<std::string> getUpdatedPageList(std::int64_t startTime){
		//to make sure we get every single page, we're going to run the same thing twice and remove all duplicates
		const auto collectionFunction = [startTime]()->std::vector<std::string>{
			std::vector<std::string> output;
			
			int pageNumber = 1;
			
			bool keepGoing = true;
			while(keepGoing){
				std::cout << "Current Page of Listing:" << pageNumber << "\n";
				curlpp::Cleanup myCleanup;
				curlpp::Easy request;
				request.setOpt<curlpp::options::Url>(website + "most-recently-edited/p/" + std::to_string(pageNumber));
				
				std::list<std::string> headers;
				headers.push_back(userAgent);
				headers.push_back("Accept: test/html");
				headers.push_back("Accept-Encoding: identity");//gzip, deflate
				headers.push_back("Accept-Language: en-US,en;q=0.9");
				request.setOpt<curlpp::options::HttpHeader>(headers);
				
				std::ostringstream os;
				curlpp::options::WriteStream ws(&os);
				request.setOpt(ws);
				request.perform();
				std::string data = os.str();
				{
					std::string pageListData;
					getData(data, "<div class=\"content-type-description\"><div class=\"list-pages-box\">", "<div class=\"pager\"><span class=\"pager-no\">", 0, pageListData);
					std::size_t start = 0;
					while(keepGoing){
						std::string pageName;
						start = getData(data, "<td style=\"vertical-align: top;\"><a href=\"/", "\"", start, pageName);
						if(start == std::string::npos){
							break;
						}
						std::string timeStamp;
						getData(data, "odate time_", " ", start, timeStamp);
						if(std::stoll(timeStamp) >= startTime){
							output.push_back(pageName);
						}
						else{
							keepGoing = false;//we started seeing pages we've already seen, so stop searching
						}
					}
				}
				pageNumber++;
			}
			return output;
		};
		std::cout << "Getting Set A\n";
		std::vector<std::string> setA = collectionFunction();
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "Getting Set B\n";
		std::vector<std::string> setB = collectionFunction();
		
		//combine the two lists and remove duplicates
		setA.insert(setA.end(), setB.begin(), setB.end());
		std::sort(setA.begin(), setA.end());
		auto uniqueEnd = std::unique(setA.begin(), setA.end());
		std::cout << "Pages that were found in one set but not the other: " << ((uniqueEnd - setA.begin()) - (setA.end()  - uniqueEnd)) << "\n";
		std::this_thread::sleep_for(std::chrono::seconds(5));
		setA.erase(uniqueEnd, setA.end());
		
		return setA;
		
	}
	
	namespace{
		struct ThreadPool{
			std::mutex threadLock;
			std::vector<std::thread> pool;
			std::stack<std::function<void(void)>> work;
			//std::atomic<std::size_t> workItems{0};
			std::atomic<bool> keepRunning{true};
			static constexpr int threadCount = 15;//15 + main thread for a nice round 16
			
			std::function<void(void)> getWork(){
				std::lock_guard<std::mutex> lock(threadLock);
				if(work.size() > 0){
					auto func = work.top();
					work.pop();
					//workItems--;
					return func;
				}
				else{
					return {};
				}
			}
			
			inline void doWorkWhile(std::function<bool(void)> checkFunc){
				while(checkFunc()){
					auto func = getWork();
					if(func != nullptr){
						func();
					}
					else{
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}
				}
			}
			
			ThreadPool(){
				for(int i = 0; i < threadCount; i++){
					pool.push_back(std::thread([this](){
						doWorkWhile([this](){return keepRunning.load();});
					}));
				}
			}
			
			~ThreadPool(){
				keepRunning = false;
				for(int i = 0; i < threadCount; i++){
					pool[i].join();
				}
				pool.clear();
			}
			
			void addWork(std::function<void(void)> newWork){
				std::lock_guard<std::mutex> lock(threadLock);
				work.push(newWork);
				//workItems++;
			}
		};
		ThreadPool threadPool;
		
		std::vector<nlohmann::json> executeCollectionOnThreads(std::vector<std::string> collection, std::function<nlohmann::json(std::string)> func){
			std::mutex outputLock;
			std::map<std::size_t, nlohmann::json> output;
			
			std::atomic<int> count{0};
			
			for(int index = 0; index < collection.size(); index++){
				std::string entry = collection[index];
				threadPool.addWork([index, entry, &count, &output, &outputLock, &func](){
					nlohmann::json out = func(entry);
					{
						std::lock_guard<std::mutex> lock(outputLock);
						output[index] = out;
					}
					count++;
				});
			}
			
			/*
			while(true){
				int num = count;
				std::cout << num << "/" << collection.size() << "   " << (static_cast<double>(num) / collection.size() * 100) << "% complete, " << threadPool.workItems << " work items\n";
				if(num == collection.size()){
					break;
				}
				std::this_thread::sleep_for(std::chrono::seconds(15));
			}
			*/
			std::size_t collectionSize = collection.size();
			threadPool.doWorkWhile([&count, collectionSize](){return count != collectionSize;});
			
			std::vector<nlohmann::json> outputVec;
			for(auto i = output.begin(); i != output.end(); i++){
				outputVec.push_back(i->second);
			}
			return outputVec;
		}
	}
	
	void downloadPageList(std::string pagesFolder, std::vector<std::string> pageList){
		std::cout << "Archiving " << pageList.size() << " pages...\n";
		
		executeCollectionOnThreads(pageList, [pagesFolder](std::string page)->nlohmann::json{
			try{
				downloadFullPageArchive(pagesFolder, page);
			}
			catch(std::exception& e){
				std::string pageFolder = pagesFolder + page + "/";
				
				std::lock_guard<std::mutex> lock(threadPool.threadLock);
				std::string error = "Error when processing page " + page + " e.what()" + e.what() + "\n";
				
				boost::filesystem::remove_all(pageFolder);
				boost::filesystem::create_directory(pageFolder);//make sure the folder is empty
			}
			return {};
		});
	}
	
	void downloadFullPageArchive(std::string pagesFolder, std::string pageName){
		std::cout << "\tArchiving page \"" << pageName << "\"...\n";
		
		std::string pageId = getPageId(pageName);
		
		std::string pageFolder = pagesFolder + pageName + "/";
		std::string filesFolder = pageFolder + "files/";
		
		if(boost::filesystem::equivalent(boost::filesystem::path(pageFolder), boost::filesystem::path(pagesFolder))){
			throw std::runtime_error("Path error on page " + pageName);
		}
		
		//make sure there isn't any stale data already in the directory
		boost::filesystem::remove_all(pageFolder);
		
		boost::filesystem::create_directory(pageFolder);
		boost::filesystem::create_directory(filesFolder);
				
		if(pageId == "" && pageExists(pageName) == false){
			std::cout << "\t\tPage " << pageName << " does not exist.\n";
			nlohmann::json page;
			page["nonExistent"] = true;
			saveJsonToFile(pageFolder + "data.json", page);
		}
		else{
			nlohmann::json page;
			page["id"] = pageId;
			page["parent"] = getPageParent(pageName);
			page["discussionId"] = getPageDiscussionId(pageName);
			page["name"] = pageName;
			page["files"] = getPageFiles(pageId);
			page["revisions"] = getPageRevisions(pageId);
			page["tags"] = getPageTags(pageName);
			page["votes"] = getPageVotes(pageId);
			
			saveJsonToFile(pageFolder + "data.json", page);
			
			std::cout << "\t\tDownloading " << page["files"].size() << " files...\n";
			
			for(auto file : page["files"]){
				std::string fileName = filesFolder + file["id"].get<std::string>();
				std::string url = file["url"].get<std::string>();
				std::cout << "\t\t\tDownloading file \"" << fileName << "\" from \"" << url << "\"...\n";
				downloadImage(fileName, url);
			}
		}
	}
	
	bool pageExists(std::string pageName){
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<curlpp::options::Url>(website + pageName + noRedirect);
		
		std::list<std::string> headers;
		headers.push_back(userAgent);
		headers.push_back("Accept: text/html; charset=utf8");
		headers.push_back("Accept-Encoding: identity");//gzip, deflate
		headers.push_back("Accept-Language: en-US,en;q=0.9");
		request.setOpt<curlpp::options::HttpHeader>(headers);
		
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		
		if(data.find("This page doesn't exist") == std::string::npos){
			return true;
		}
		else{
			return false;
		}
	}
	
	std::string getPageId(std::string pageName){
		
		std::cout << "\t\tGetting Page ID...\n";
	
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<curlpp::options::Url>(website + pageName + noRedirect);
		
		std::list<std::string> headers;
		headers.push_back(userAgent);
		headers.push_back("Accept: text/html; charset=utf8");
		headers.push_back("Accept-Encoding: identity");//gzip, deflate
		headers.push_back("Accept-Language: en-US,en;q=0.9");
		request.setOpt<curlpp::options::HttpHeader>(headers);
		
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		
		std::string pageId;
		getData(data, "WIKIREQUEST.info.pageId = ", ";", 0, pageId);
		trimString(pageId);
		return pageId;
	}
	
	std::string getPageParent(std::string pageName){
	
		std::cout << "\t\tGetting Page Parent...\n";
	
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<curlpp::options::Url>(website + pageName + noRedirect);
		
		//request.setOpt<curlpp::options::Verbose>(true);
		
		std::list<std::string> headers;
		headers.push_back(userAgent);
		headers.push_back("Accept: text/html; charset=utf8");
		headers.push_back("Accept-Encoding: identity");//gzip, deflate
		headers.push_back("Accept-Language: en-US,en;q=0.9");
		request.setOpt<curlpp::options::HttpHeader>(headers);
		
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		
		std::string breadCrumbs;
		std::size_t breadCrumbsPos = getData(data, "<div id=\"breadcrumbs\">", "</div>", 0, breadCrumbs);
		if(breadCrumbsPos == std::string::npos){
		    return "";
		}
		else{//the parent is going to be the last thing in the breadcrumb list, so just get that
		    std::string output;
		    std::size_t pos = 0;
		    while(true){
		        std::string parent;
		        pos = getData(breadCrumbs, "<a href=\"/", "\"", pos, parent);
		        if(pos == std::string::npos){
		            return output;
		        }
		        else{
		            output = parent;
		        }
		    }
		}
	}
	
	std::string getPageDiscussionId(std::string pageName){
	
		std::cout << "\t\tGetting Page Discussion ID...\n";
		
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<curlpp::options::Url>(website + pageName + noRedirect);
		
		std::list<std::string> headers;
		headers.push_back(userAgent);
		headers.push_back("Accept: text/html; charset=utf8");
		headers.push_back("Accept-Encoding: identity");//gzip, deflate
		headers.push_back("Accept-Language: en-US,en;q=0.9");
		request.setOpt<curlpp::options::HttpHeader>(headers);
		
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		
		std::string threadId;
		getData(data, "/forum/t-", "/", data.find("<div id=\"page-options-bottom\""), threadId);
		trimString(threadId);
		return threadId;
	}
	
	nlohmann::json getPageRevisions(std::string pageId){
	
		std::cout << "\t\tGetting Page Revisions...\n";
	
		nlohmann::json output = nlohmann::json::array();
		
		std::string rawRevisionData = performAjaxRequest("history/PageRevisionListModule",
				{{"page_id", pageId}, {"options", "{\"all\":true}"}, {"perpage", "999999"}, {"page", "1"}}
			)["body"];//wikidot doesn't actually have a cap for the "perpage" thing so just make it really big
		
		std::size_t pos = rawRevisionData.find("</tr>");//start at the end of the first one
		std::size_t count = 0;
		{
			std::size_t posB = pos;
			while(true){
				std::string revisionData;
				posB = getData(rawRevisionData, "<tr", "</tr>", posB, revisionData);
				if(posB == std::string::npos){
					break;
				}
				else{
					count++;
				}
			}
		}
		
		std::cout << "\t\t\tCollected " << count << " Revisions\n";
		
		while(true){
			std::string revisionData;
			pos = getData(rawRevisionData, "<tr", "</tr>", pos, revisionData);
			if(pos == std::string::npos){
				break;
			}
			else{
				std::string revisionId;
				getData(revisionData, "showVersion(", ")", 0, revisionId);
				
				//std::cout << "\t\t\tCollecting Revision #" << output.size() << "/" << count << "...\n";
				
				nlohmann::json revision;
				revision["id"] = revisionId;
				
				std::string changeType;
				getData(revisionData, "<span class=\"spantip\" title=\"", "\"", 0, changeType);
				revision["changeType"] = changeType;
				
				std::string changeMessage;
				getData(revisionData, "<td style=\"font-size: 90%\">", "</td>", 0, changeMessage);
				revision["changeMessage"] = decodeHtmlEntities(changeMessage);//make sure to decode the html entities
				
				//revision["sourceCode"] = getRevisionSource(revisionId);
				
				output.push_back(revision);
			}
		}
		
		std::reverse(output.begin(), output.end());//reverse the order so the page revisions go from oldest to newest
		std::vector<std::string> revisionIds;
		for(const auto& entry : output){
			revisionIds.push_back(entry["id"].get<std::string>());
		}
		std::atomic<bool> error{false};
		std::vector<nlohmann::json> sourceCode = executeCollectionOnThreads(revisionIds, [&error](std::string revisionId)->nlohmann::json{
			try{
				std::cout << "\t\t\t\tGetting Revision ID #" << revisionId << "\n";
				nlohmann::json output = getRevisionData(revisionId);
				output["sourceCode"] = getRevisionSource(revisionId);
				return output;
			}
			catch(std::exception& e){
				error = true;
				return{};
			}
		});
		if(error){
			throw std::runtime_error("Error when downloading page revision for page ID:" + pageId);
		}
		for(std::size_t i = 0; i < output.size(); i++){
			output[i]["sourceCode"] = sourceCode[i]["sourceCode"].get<std::string>();
			output[i]["title"] = sourceCode[i]["title"].get<std::string>();
			output[i]["authorId"] = sourceCode[i]["authorId"].get<std::string>();
			output[i]["timeStamp"] = sourceCode[i]["timeStamp"].get<std::string>();
		}
		
		std::cout << "\t\t\tGot " << output.size() << " Revisions           \n"; 
		
		return output;
	}
	
	nlohmann::json getRevisionData(std::string revisionId){
		nlohmann::json revision;
		
		revision["id"] = revisionId;
		
		nlohmann::json revisionReply = performAjaxRequest("history/PageVersionModule", {{"revision_id", revisionId}});
		
		if(revisionReply["title"].is_null()){
			revision["title"] = "";
		}
		else{
			revision["title"] = revisionReply["title"].get<std::string>();
		}
		
		std::string rawRevisionSource = revisionReply["body"].get<std::string>();
		
		std::string authorId;
		getData(rawRevisionSource, "userInfo(", ")", 0, authorId);
		if(authorId == ""){
			authorId = "deleted";
		}
		revision["authorId"] = authorId;
		
		std::string timeStamp;
		getData(rawRevisionSource, "time_", " ", 0, timeStamp);
		revision["timeStamp"] = timeStamp;
		
		return revision;
	}
	
	std::string getRevisionSource(std::string revisionId){
		std::string rawSource = performAjaxRequest("history/PageSourceModule", {{"revision_id", revisionId}})["body"].get<std::string>();
		
		std::size_t start = rawSource.find("<div class=\"page-source\">") + 26;
		std::string source = rawSource.substr(start, rawSource.size() - start - 7);
		
		//now we gotta decode the html source into the original source code
		replaceAll(source, "<br />", "");//remove the extra line breaks
		source = decodeHtmlEntities(source);//and decode the html entities too
		
		return source;
	}
	
	nlohmann::json getPageFiles(std::string pageId){
	
		std::cout << "\t\tGetting Page Files...\n";
		
		nlohmann::json output = nlohmann::json::array();
		
		std::string rawFileData = performAjaxRequest("files/PageFilesModule", {{"page_id", pageId}})["body"].get<std::string>();
		
		std::size_t pos = rawFileData.find("</tr>");//start at the end of the first one
		while(true){
			std::string fileData;
			pos = getData(rawFileData, "<tr", "</tr>", pos, fileData);
			if(pos == std::string::npos){
				break;
			}
			else{
				std::string fileId;
				getData(fileData, "toggleFileOptions(", ")", 0, fileId);
				output.push_back(getFileData(fileId));
			}
		}
		
		return output;
	}
	
	nlohmann::json getFileData(std::string fileId){
		std::string rawFileData = performAjaxRequest("files/FileInformationWinModule", {{"file_id", fileId}})["body"].get<std::string>();
		
		nlohmann::json file;
		
		file["id"] = fileId;
		
		std::string name;
		getData(rawFileData, "<strong>", "</strong>", 0, name);
		file["name"] = decodeHtmlEntities(name);//make sure to decode the html in the name
		
		std::string url;
		///percent decoding the url might be necessary, but probably not
		getData(rawFileData, "<a href=\"", "\"", 0, url);
		file["url"] = url;
		
		std::string authorId;
		getData(rawFileData, "userInfo(", ")", 0, authorId);
		if(authorId == ""){
			authorId = "deleted";
		}
		file["authorId"] = authorId;
		
		std::string timeStamp;
		getData(rawFileData, "time_", " ", 0, timeStamp);
		file["timeStamp"] = timeStamp;
		
		std::string description;
		getData(rawFileData, "<td>", "</td>", rawFileData.find("Comment:"), description);
		description = trimString(description);//we need a trim string here, which shouldn't have any bad side effects
		file["description"] = decodeHtmlEntities(description);//make sure to decode the html too
		
		return file;
	}
	
	nlohmann::json getPageTags(std::string pageName){
		
		std::cout << "\t\tGetting Page Tags...\n";
		
		//I was doing to use the following module, but it turns out that wikidot doesn't actually let me unless I'm an admin
		
			//I'd like to point out that wikidot normally uses "page_id" but this module uses "pageId" because of course it does
			//std::ofstream("raw.html") << performAjaxRequest("pageTags/PageTagsModule", {{"pageId", pageId}}).dump(4);
		
		//I'm not an admin, so I'm just gonna have to get the tags out of the html
		
		std::string rawPage;
		{
			curlpp::Cleanup myCleanup;
			curlpp::Easy request;
			request.setOpt<curlpp::options::Url>(website + pageName + noRedirect);
			
			std::list<std::string> headers;
			headers.push_back(userAgent);
			headers.push_back("Accept: test/html");
			headers.push_back("Accept-Encoding: identity");//gzip, deflate
			headers.push_back("Accept-Language: en-US,en;q=0.9");
			request.setOpt<curlpp::options::HttpHeader>(headers);
			
			std::ostringstream os;
			curlpp::options::WriteStream ws(&os);
			request.setOpt(ws);
			request.perform();
			rawPage = os.str();
		}
		
		nlohmann::json tags = nlohmann::json::array();
		
		std::size_t pos = rawPage.find("<div class=\"page-tags\">");
		while(true){
			std::string tag;
			pos = getData(rawPage, "/system:page-tags/tag/", "#pages", pos, tag);
			if(pos == std::string::npos){
				break;
			}
			tags.push_back(tag);
		}
		
		return tags;
	}
	
	nlohmann::json getPageVotes(std::string pageId){
	
		std::cout << "\t\tGetting Page Votes...\n";
	
		nlohmann::json votes = nlohmann::json::array();
		
		std::string rawVotes = performAjaxRequest("pagerate/WhoRatedPageModule", {{"pageId", pageId}})["body"].get<std::string>();
		
		std::size_t pos = 0;
		while(true){
			std::string voteBlock;
			pos = getData(rawVotes, "<span class=\"", "</span><br/>", pos, voteBlock);
			if(pos == std::string::npos){
				break;
			}
			nlohmann::json vote;
			std::string authorId;
			getData(voteBlock, "userInfo(", ")", 0, authorId);
			if(authorId == ""){
				authorId = "deleted";
			}
			vote["authorId"] = authorId;
			
			if(voteBlock.find("+              ") != std::string::npos){
				vote["voteType"] = true;
			}
			else if(voteBlock.find("-              ") != std::string::npos){
				vote["voteType"] = false;
			}
			else{
				throw std::runtime_error("Error when getting page votes");
			}
			
			votes.push_back(vote);
		}
		
		std::cout << "\t\t\t Got " << votes.size() << " Votes...\n";
		
		return votes;
	}
	
	namespace{
		size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream){
		  std::string* file = static_cast<std::string*>(stream);
		  file->append(static_cast<char*>(ptr), size * nmemb);
		  return size * nmemb;
		}
	}
	
	void downloadImage(std::string fileName, std::string fileUrl){
		try{
			
			/*
			curlpp::Cleanup myCleanup;
			curlpp::Easy request;
			request.setOpt<Url>(fileUrl);
			
			std::list<std::string> headers;
			headers.push_back(userAgent);
			headers.push_back("Accept: image");
			
			headers.push_back("Accept-Encoding: identity");//gzip, deflate
			headers.push_back("Accept-Language: en-US,en;q=0.9");
			request.setOpt<HttpHeader>(headers);
			
			std::ofstream imageFile(fileName);
			curlpp::options::WriteStream ws(&imageFile);
			request.setOpt(ws);
			request.perform();
			imageFile.close();
			*/
			
			{
				CURL *curl_handle;

				curl_global_init(CURL_GLOBAL_ALL);

				/* init the curl session */ 
				curl_handle = curl_easy_init();

				/* set URL to get here */ 
				curl_easy_setopt(curl_handle, CURLOPT_URL, fileUrl.c_str());

				/* Switch on full protocol/debug output while testing */ 
				curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

				/* disable progress meter, set to 0L to enable and disable debug output */ 
				curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
				
				//follow redirect ways
				curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

				/* send all data to this function  */ 
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
				
				std::string imageData;
				/* write the page body to this file handle */ 
				curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(&imageData));

				/* get it! */ 
				curl_easy_perform(curl_handle);

				/* cleanup curl stuff */ 
				curl_easy_cleanup(curl_handle);
				
				std::ofstream imageFile(fileName);
				imageFile << imageData;
				imageFile.close();
			}
			
		}
		catch(std::exception& e){
			std::cout << "\t\t\t\tError when downloading image: \"" << e.what() << "\", continuing...\n";
		}
	}
	
	nlohmann::json getForumGroups(){
		nlohmann::json forumGroups = nlohmann::json::array();
		
		std::string rawForum = performAjaxRequest("forum/ForumStartModule", {{"hidden", "true"}})["body"].get<std::string>();
		
		std::size_t pos = 0;
		while(true){
			std::string rawGroup;
			pos = getData(rawForum, "forum-group", "</table>", pos, rawGroup);
			if(pos == -1){
				break;
			}
			nlohmann::json group;
			{
				{
					std::string title;
					getData(rawGroup, "class=\"title\">", "<", 0, title);
					group["title"] = decodeHtmlEntities(title);//make sure to decode html entities
					
					std::string description;
					getData(rawGroup, "class=\"description\">", "<", 0, description);
					group["description"] = decodeHtmlEntities(description);//make sure to decode html entities
				}
				
				group["categories"] = nlohmann::json::array();
				std::size_t catPos = 0;
				while(true){
					std::string rawCategory;
					catPos = getData(rawGroup, "class=\"name\"", "</tr>", catPos, rawCategory);
					if(catPos == -1){
						break;
					}
					nlohmann::json category;
					{
						{
							std::string id;
							std::size_t idPos = getData(rawCategory, "/forum/c-", "/", 0, id);
							category["id"] = id;
							
							std::string title;
							getData(rawCategory, ">", "<", idPos, title);
							category["title"] = decodeHtmlEntities(title);//make sure to decode html entities
							
							std::string description;
							getData(rawCategory, "class=\"description\">", "<", 0, description);
							category["description"] = decodeHtmlEntities(description);//make sure to decode html entities
						}
					}
					group["categories"].push_back(category);
				}
			}
			forumGroups.push_back(group);
		}
		return forumGroups;
	}
	
	std::vector<std::string> getThreadListForForumCategory(std::string categoryId){
		std::vector<std::string> output;
		
		int pageNumber = 0;
		while(true){
			std::cout << "Current Page of Listing:" << pageNumber << "\n";
			std::string rawList = performAjaxRequest("forum/ForumViewCategoryModule", {{"c", categoryId}, {"p", std::to_string(pageNumber)}})["body"].get<std::string>();
			int countOnPage = 0;
			std::size_t pos = 0;
			while(true){
				pos = rawList.find("<div class=\"title\">", pos);
				if(pos == std::string::npos){
					break;
				}
				std::string id;
				pos = getData(rawList, "/forum/t-", "/", pos, id);
				output.push_back(id);
				countOnPage++;
			}
			if(countOnPage == 0){
				std::cout << "Current Listing Page is Empty; Reached End of Pages.\n";
				break;
			}
			pageNumber++;
		}
		
		return output;
	}
	
	std::vector<std::string> getThreadListForAllCategories(nlohmann::json forumGroups){
		std::vector<std::string> output;
		for(auto group : forumGroups){
			for(auto category : group["categories"]){
				std::cout << "\nGetting thread listing for category " << category["id"].get<std::string>() << " \"" << category["title"].get<std::string>() << "\"...\n";
				std::vector<std::string> sublist = getThreadListForForumCategory(category["id"].get<std::string>());
				output.insert(output.end(), sublist.begin(), sublist.end());
			}
		}
		return output;
	}
	
	void downloadThreadList(std::string threadsFolder, std::vector<std::string> threadList){
		std::cout << "Archiving " << threadList.size() << " threads...\n";
		for(int i = 0; i < threadList.size(); i++){
			downloadFullThreadArchive(threadsFolder, threadList[i]);
			std::cout << i + 1 << "/" << threadList.size() << "   " << (static_cast<double>(i + 1) / threadList.size() * 100) << "% complete\n";
		}
	}
	
	void downloadFullThreadArchive(std::string threadsFolder, std::string threadId){
		std::cout << "\tArchiving thread \"" << threadId << "\"...\n";
		
		std::string threadFolder = threadsFolder + threadId + "/";
		
		//make sure there isn't any stale data already in the directory
		boost::filesystem::remove_all(threadFolder);
		boost::filesystem::create_directory(threadFolder);
		
		std::string rawThread = performAjaxRequest("forum/ForumViewThreadModule", {{"t", threadId}, {"pageNo", "1"}})["body"].get<std::string>();
		
		nlohmann::json thread;
		{
			std::string categoryId;
			getData(rawThread, "/forum/c-", "/", 0, categoryId);
			thread["categoryId"] = categoryId;
			
			std::string threadId;
			getData(rawThread, "/feed/forum/t-", ".xml", 0, threadId);
			thread["id"] = threadId;
			
			std::string title;
			getData(rawThread, " ", "</div>", rawThread.rfind("&raquo;", rawThread.find("<div class=\"description-block well\">")), title);
			trimString(title);
			thread["title"] = decodeHtmlEntities(title);
			
			std::string timeStamp;
			getData(rawThread, "odate time_", " ", 0, timeStamp);
			thread["timeStamp"] = timeStamp;
			
			std::string authorId;
			{
				std::string statisticsBlock;
				getData(rawThread, "<div class=\"description-block well\">", ".xml", 0, statisticsBlock);
				
				if(statisticsBlock.find("<span class=\"printuser\">Wikidot</span>") != std::string::npos){
					authorId = "wikidot";
				}
				else{
					getData(statisticsBlock, "WIKIDOT.page.listeners.userInfo(", ")", 0, authorId);
					if(authorId == ""){
						authorId = "deleted";
					}
				}
			}
			thread["authorId"] = authorId;
			
			{
				std::size_t startPos = rawThread.find("</div>", rawThread.find(".xml")) + std::string("</div>").size();
				std::size_t endPos = rawThread.rfind("</div>", rawThread.find("<div class=\"options\">"));
				std::string description = rawThread.substr(startPos, endPos - startPos);
				trimString(description);
				thread["description"] = description;
			}
		}
		
		//now get the actual posts
		//to do that we need to get all the posts from every page before actually parsing them all
		int totalPages = 1;
		if(rawThread.find("<div class=\"pager\">") != std::string::npos){
			std::string totalPagesRaw;
			getData(rawThread, "<span class=\"pager-no\">page 1 of ", "</span>", 0, totalPagesRaw);
			totalPages = std::stoi(totalPagesRaw);
		}
		
		std::string allRawPosts;
		
		int currentPage = 1;
		while(true){
			std::cout << "\t\tCurrent Page of Thread Posts: " << currentPage << " out of " << totalPages << "\n";
			
			std::string rawPostBlock = performAjaxRequest("forum/ForumViewThreadPostsModule", {{"t", threadId}, {"pageNo", std::to_string(currentPage)}, {"order", ""}})["body"].get<std::string>();
			
			//remove pagers
			while(rawPostBlock.find("<div class=\"pager\">") != std::string::npos){
				std::size_t startErase = rawPostBlock.find("<div class=\"pager\">");
				std::size_t endErase = rawPostBlock.find("</span></div>") + std::string("</span></div>").size();
				rawPostBlock.erase(startErase, endErase - startErase);
			}
			
			allRawPosts += rawPostBlock;
			
			currentPage++;
			if(currentPage > totalPages){
				break;
			}
		}
		
		//I know this code is awful and this isn't how html parsing should work, but
		//this whole function is really just a placeholder right now anyways until 
		//we get to use the actual wikidot api to do this mess, so it's just gonna have to work for now
		nlohmann::json divTreeRoot;
		{
			std::vector<nlohmann::json> divStack;
			const auto getNewDiv = [](){
				nlohmann::json newDiv;
				newDiv["classes"] = "";
				newDiv["content"] = "";
				newDiv["divs"] = nlohmann::json::array();
				return newDiv;
			};
			divStack.push_back(getNewDiv());
			
			std::size_t pos = 0;
			while(true){
				std::size_t nextDiv = allRawPosts.find("<div", pos);
				std::size_t nextDivEnd = allRawPosts.find("</div>", pos);
				
				if(nextDivEnd == std::string::npos){
					break;
				}
				
				std::size_t newContent = std::min(nextDiv, nextDivEnd);
				std::string content = divStack.back()["content"].get<std::string>() + " " + allRawPosts.substr(pos, newContent - pos);
				divStack.back()["content"] = trimString(content);
				pos = newContent;
				
				if(nextDiv < nextDivEnd){
					pos += std::string("<div").size();
					std::size_t classesEnd = allRawPosts.find(">", pos);
					nlohmann::json newDiv = getNewDiv();
					std::string classes = allRawPosts.substr(pos, classesEnd - pos);
					newDiv["classes"] = trimString(classes);
					pos = classesEnd + std::string(">").size();
					divStack.push_back(newDiv);
				}
				else{
					nlohmann::json div = divStack.back();
					divStack.pop_back();
					/*
					if(divStack.size() == 0){
						divTreeRoot = div;
						break;
					}
					*/
					divStack.back()["divs"].push_back(div);
					pos += std::string("</div>").size();
				}
			}
			divTreeRoot = divStack.back();
		}
		
		thread["posts"] = getThreadPostReplies(divTreeRoot["divs"], 0);
		
		std::ofstream(threadFolder + "data.json") << thread.dump(4);
	}
	
	nlohmann::json getThreadPostReplies(const nlohmann::json& divTree, int offset){
		nlohmann::json posts = nlohmann::json::array();
		
		for(int i = offset; i < divTree.size(); i++){
			posts.push_back(getThreadPost(divTree[i]));
		}
		
		return posts;
	}
	
	nlohmann::json getThreadPost(const nlohmann::json& divTree){
		nlohmann::json post;
		
		std::string postId;
		getData(divTree["classes"].get<std::string>(), "id=\"fpc-", "\"", 0, postId);
		post["postId"] = postId;
		
		std::string authorId;
		getData(divTree["divs"][0]["divs"][0]["divs"][0]["divs"][2]["content"].get<std::string>(), "userInfo(", ")", 0, authorId);
		if(authorId == ""){
			authorId = "deleted";
		}
		post["authorId"] = authorId;
		
		std::string title;
		title = divTree["divs"][0]["divs"][0]["divs"][0]["divs"][1]["content"].get<std::string>();
		post["title"] = decodeHtmlEntities(title);
		
		std::string timeStamp;
		getData(divTree["divs"][0]["divs"][0]["divs"][0]["divs"][2]["content"].get<std::string>(), "odate time_", " ", 0, timeStamp);
		post["timeStamp"] = timeStamp;
		
		std::string content;
		content = divTree["divs"][0]["divs"][0]["divs"][1]["content"].get<std::string>();
		post["content"] = content;
		
		post["posts"] = getThreadPostReplies(divTree["divs"], 1);
 		
		return post;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}
