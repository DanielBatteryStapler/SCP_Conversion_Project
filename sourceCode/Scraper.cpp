#include "Scraper.h"

#include "Parser.h"

#include <boost/filesystem.hpp>

using namespace curlpp::options;

const std::string userAgent = "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.89 Safari/537.36";
//I don't want wikidot to know that this is a bot, so I'm going to use the useragent of a real browser(aka my browser and computer)
//Is it unethical to scrape a site while acting like I'm not a bot? maybe...

std::vector<std::string> Scraper::getPageList(){
	std::vector<std::string> output;
	
	int pageNumber = 1;
	
	while(true){
		std::cout << "Current Page of Listing:" << pageNumber << "\n";
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<Url>("http://www.scp-wiki.net/system:list-all-pages/p/" + std::to_string(pageNumber));
		
		std::list<std::string> headers;
		headers.push_back(userAgent);
		headers.push_back("Accept: test/html");
		headers.push_back("Accept-Encoding: identity");//gzip, deflate
		headers.push_back("Accept-Language: en-US,en;q=0.9");
		request.setOpt<HttpHeader>(headers);
		
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

void Scraper::getRawPageData(std::string page, std::string& pageId, std::string& pageTitle){
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;
	request.setOpt<Url>("http://www.scp-wiki.net/" + page);
	
	std::list<std::string> headers;
	headers.push_back(userAgent);
	headers.push_back("Accept: test/html");
	headers.push_back("Accept-Encoding: identity");//gzip, deflate
	headers.push_back("Accept-Language: en-US,en;q=0.9");
	request.setOpt<HttpHeader>(headers);
	
	std::ostringstream os;
	curlpp::options::WriteStream ws(&os);
	request.setOpt(ws);
	request.perform();
	std::string data = os.str();
	
	getData(data, "<div id=\"page-title\">", "</div>", 0, pageTitle);
	trimString(pageTitle);
	
	getData(data, "WIKIREQUEST.info.pageId = ", ";", 0, pageId);
	trimString(pageId);
}

void Scraper::getRawPageSource(std::string pageId, std::string& pageSource){
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;
	request.setOpt<Url>("http://www.scp-wiki.net/ajax-module-connector.php");
	
	std::list<std::string> headers;
	headers.push_back("X-Requested-With: XMLHttpRequest");
	headers.push_back(userAgent);
	headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	headers.push_back("Accept: application/json");
	headers.push_back("Accept-Encoding: identity");
	headers.push_back("Accept-Language: en-US,en;q=0.9");
	request.setOpt<HttpHeader>(headers);
	
	std::string body = "moduleName=" + percentEncode("viewsource/ViewSourceModule") + "&page_id=" + pageId;
	
	request.setOpt<PostFields>(body);
	request.setOpt<PostFieldSize>(body.length());
	
	std::ostringstream os;
	curlpp::options::WriteStream ws(&os);
	request.setOpt(ws);
	request.perform();
	std::string data = os.str();

	nlohmann::json page = nlohmann::json::parse(data);
	
	std::string pageBody = page["body"];
	
	std::size_t start = pageBody.find("<div class=\"page-source\">") + 26;
	
	pageBody = pageBody.substr(start, pageBody.size() - start - 7);
	
	replaceAll(pageBody, "\n", "");
	pageBody += '\n';//page should always end in a newline
	
	//unescape the html that wikidot decides to escape
	replaceAll(pageBody, "<br />", "\n");
	replaceAll(pageBody, "&gt;", ">");
	replaceAll(pageBody, "&lt;", "<");
	replaceAll(pageBody, "&quot;", "\"");
	replaceAll(pageBody, "&apos;", "'");
	replaceAll(pageBody, "&#039;", "'");
	replaceAll(pageBody, "&nbsp;", " ");
	replaceAll(pageBody, "&amp;", "&");
	
	pageSource = pageBody;
	trimString(pageSource);
}

void Scraper::downloadPageSource(std::string page){
	std::string pageId;
	std::string pageTitle;
	getRawPageData(page, pageId, pageTitle);
	
	std::string pageSource;
	getRawPageSource(pageId, pageSource);
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		
		boost::filesystem::create_directory("websitePages");
		boost::filesystem::create_directory("websitePages/" + page);
		std::ofstream file("websitePages/" + page + "/" + page + ".wikidotSource");
		file << 
		"[!--\n"
		"<SCP_Conversion_Project_Page_Info_Header>\n"
		"    Project Homepage: https://github.com/DanielBatteryStapler/SCP_Conversion_Project\n"
		"    <Page_Name>" << page << "</Page_Name>\n"
		"    <Page_Id>" << pageId << "</Page_Id>\n"
		"    <Page_Title>" << pageTitle << "</Page_Title>\n"
		"    <Full_Page_URL>" << "http://www.scp-wiki.net/" << page << "</Full_Page_URL>\n"
		"    <UTC_TimeStamp_Of_Page_Download>" << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "</UTC_TimeStamp_Of_Page_Download>\n"
		"</SCP_Conversion_Project_Page_Info_Header>\n"
		"--]\n\n"//add the custom header that adds some extra information to the article for archival purposes as well as passing some info for conversion to html and stuff
		<< pageSource;//add the actual page source
		file.close();
	}
}

void Scraper::convertPageSourceToHtml(std::string page, std::string output, bool printFriendly, bool downloadImages){
	std::string pageSource;
	{
		std::ifstream file("websitePages/" + page + "/" + page + ".wikidotSource");
		std::stringstream ss;
		ss << file.rdbuf();
		pageSource = ss.str();
	}
	std::string pageTitle;
	std::string fullPageUrl;
	{//get additional page info from the SCP_Conversion_Project_Page_Info_Header
		std::string pageInfoHeader;
		getData(pageSource, "<SCP_Conversion_Project_Page_Info_Header>", "</SCP_Conversion_Project_Page_Info_Header>", 0, pageInfoHeader);
		
		getData(pageInfoHeader, "<Page_Title>", "</Page_Title>", 0, pageTitle);
		getData(pageInfoHeader, "<Full_Page_URL>", "</Full_Page_URL>", 0, fullPageUrl);
	}
	std::vector<Token> tokenizedArticle = Parser::tokenizeArticle(pageSource);
	Statement statementizedArticle = Parser::statementizeArticle(tokenizedArticle, pageTitle);
	
	std::string pageHtml = Parser::convertToHtml(statementizedArticle, printFriendly, (fullPageUrl=="local")?false:downloadImages, "http://scp-wiki.wdfiles.com/local--files/" + page + "/", "websitePages/" + page + "/");
	{
		std::ofstream file("websitePages/" + page + "/" + output);
		file << pageHtml;
		file.close();
	}
}

void Scraper::convertPageHtmlToPdf(std::string page, std::string input, std::string output){
	std::string command = "./wkhtmltopdf --zoom 1.10 -s Letter \"websitePages/" + page + "/" + input + "\" \"websitePages/" + page + "/" + output + "\"";
	std::cout << "Running: '" << command << "'\n";
	std::system(command.c_str());
	//yes, I know that using std::system is usually not a good idea, but this is just a helpful little application that convert SCP articles
	//if somebody really wanted to break this, they would just replace this executable, not screw around with wkhtmltopdf for no reason
}

namespace{
	size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream){
	  std::string* file = static_cast<std::string*>(stream);
	  file->append(static_cast<char*>(ptr), size * nmemb);
	  return size * nmemb;
	}
}

void Scraper::downloadImage(std::string image, ParserConvertData& convertData){
	if(convertData.downloadImages){
		try{
			std::cout << "Downloading Image \"" << image << "\"...\n";
			
			/*
			curlpp::Cleanup myCleanup;
			curlpp::Easy request;
			request.setOpt<Url>(convertData.imageUrlPath + image);
			
			std::list<std::string> headers;
			headers.push_back(userAgent);
			headers.push_back("Accept: image");
			
			headers.push_back("Accept-Encoding: identity");//gzip, deflate
			headers.push_back("Accept-Language: en-US,en;q=0.9");
			request.setOpt<HttpHeader>(headers);
			
			std::ofstream imageFile(convertData.imageSavePath + percentEncode(image));
			curlpp::options::WriteStream ws(&imageFile);
			request.setOpt(ws);
			request.perform();
			imageFile.close();
			imageFile << request;
			*/
			
			{
				
				std::string url;
				if(image.find("http://") == std::string::npos && image.find("https://") == std::string::npos){
					url = convertData.imageUrlPath + image;
				}
				else{
					url = image;
				}
				
				CURL *curl_handle;

				curl_global_init(CURL_GLOBAL_ALL);

				/* init the curl session */ 
				curl_handle = curl_easy_init();

				/* set URL to get here */ 
				curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

				/* Switch on full protocol/debug output while testing */ 
				curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

				/* disable progress meter, set to 0L to enable and disable debug output */ 
				curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
				
				//follow redirect ways
				curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

				/* send all data to this function  */ 
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
				
				std::cout << "Downloading Url \"" << url << "\"\n";
				std::string imageData;
				/* write the page body to this file handle */ 
				curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(&imageData));

				/* get it! */ 
				curl_easy_perform(curl_handle);

				/* cleanup curl stuff */ 
				curl_easy_cleanup(curl_handle);
				
				std::ofstream imageFile(convertData.imageSavePath + percentEncode(image));
				imageFile << imageData;
				imageFile.close();
			}
			
			std::cout << "Finished Downloading \"" << image << "\".\n";
		}
		catch(std::exception& e){
			std::cout << "\t\t\t\tError when downloading image: \"" << e.what() << "\", continuing...\n";
		}
	}
}

/*
//these two functions work, I just have no need for them
std::string Scraper::getLatestRevisionId(std::string pageId){
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;
	request.setOpt<Url>("http://www.scp-wiki.net/ajax-module-connector.php");
	
	std::list<std::string> headers;
	headers.push_back("X-Requested-With: XMLHttpRequest");
	headers.push_back(userAgent);
	headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	
	headers.push_back("Accept: test/html");
	
	headers.push_back("Accept-Encoding: identity");
	headers.push_back("Accept-Language: en-US,en;q=0.9");
	request.setOpt<HttpHeader>(headers);
	
	std::string body = "moduleName=" + percentEncode("history/PageRevisionListModule") + "&options=" + percentEncode("{\"all\":true}") + "&page=1&page_id=" + pageId + "&perpage=1";
	
	request.setOpt<PostFields>(body);
	request.setOpt<PostFieldSize>(body.length());
	
	std::ostringstream os;
	curlpp::options::WriteStream ws(&os);
	request.setOpt(ws);
	request.perform();
	std::string data = os.str();
	
	std::string revisionId;
	getData(data, "showSource(", ")", 0, revisionId);
	return revisionId;
}

//again, it works, but I don't think I'll need it
std::string Scraper::getRawRevisionSource(std::string revisionId){
	curlpp::Cleanup myCleanup;
	curlpp::Easy request;
	request.setOpt<Url>("http://www.scp-wiki.net/ajax-module-connector.php");
	
	std::list<std::string> headers;
	headers.push_back("X-Requested-With: XMLHttpRequest");
	headers.push_back(userAgent);
	headers.push_back("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	headers.push_back("Accept: application/json");
	headers.push_back("Accept-Encoding: identity");
	headers.push_back("Accept-Language: en-US,en;q=0.9");
	request.setOpt<HttpHeader>(headers);
	
	std::string body = "moduleName=" + percentEncode("history/PageSourceModule") + "&revision_id=" + revisionId;
	
	request.setOpt<PostFields>(body);
	request.setOpt<PostFieldSize>(body.length());
	
	std::ostringstream os;
	curlpp::options::WriteStream ws(&os);
	request.setOpt(ws);
	request.perform();
	std::string data = os.str();

	nlohmann::json page = nlohmann::json::parse(data);
	
	std::string pageBody = page["body"];
	
	std::size_t start = pageBody.find("<div class=\"page-source\">") + 26;
	
	pageBody = pageBody.substr(start, pageBody.size() - start - 7);
	
	replaceAll(pageBody, "\n", "");
	pageBody += '\n';//page should always end in a newline
	
	//unescape the html that wikidot decides to escape
	replaceAll(pageBody, "<br />", "\n");
	replaceAll(pageBody, "&gt;", ">");
	replaceAll(pageBody, "&lt;", "<");
	replaceAll(pageBody, "&quot;", "\"");
	replaceAll(pageBody, "&apos;", "'");
	replaceAll(pageBody, "&#039;", "'");
	replaceAll(pageBody, "&nbsp;", " ");
	replaceAll(pageBody, "&amp;", "&");
	
	return pageBody;
}
*/









