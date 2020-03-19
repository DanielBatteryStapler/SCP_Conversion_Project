#include "Website.hpp"

#include "../Config.hpp"

#include <sstream>

#include "../Parser/HTMLConverter.hpp"
#include "../Parser/DatabaseUtil.hpp"

void Website::run(){
	std::string domainName = Config::getWebsiteDomainName();
	std::string socket = Config::getFastCGISocket();
	
	std::cout << "Starting Website on domain " << domainName << " with FastCGI on socket " << socket << "\n";
	
	Gateway::setup(domainName);
	Gateway::run(socket, threadProcess);
}

void Website::threadProcess(Gateway::ThreadContext threadContext){
	std::cout << "Starting thread #" << threadContext.threadIndex << "...\n";
	while(true){
		Gateway::RequestContext context = threadContext.getNextRequest();
		
		Website::Context websiteContext;
		websiteContext.db = Database::connectToDatabase(Config::getProductionDatabaseName());
		
		if(context.shouldShutdown){
			std::cout << "Stopping thread #" << threadContext.threadIndex << "...\n";
			return;
		}
		else{
			std::vector<std::string> uri = splitUri(context.getUriString());
			
			std::cout << "Request for URI = [";
			for(std::size_t i = 0; i < uri.size(); i++){
				std::cout << uri[i];
				if(i < uri.size() - 1){
					std::cout << ", ";
				}
			}
			std::cout << "]\n";
			
			handleUri(context, websiteContext, uri);
			
			threadContext.finishRequest(std::move(context));
		}
	}
}


std::vector<std::string> Website::splitUri(std::string uri){
	std::vector<std::string> output;
	std::stringstream uriStream(uri);
	
	std::string temp;
	while(std::getline(uriStream, temp, '/')){
		if(temp.size() != 0){
			output.push_back(urlDecode(temp));
		}
	}
	
	return output;
}

void Website::handleUri(Gateway::RequestContext& reqCon, Website::Context& webCon, std::vector<std::string> uri){
	
	bool give404 = true;
	
	if(uri.size() == 0){
		std::optional<Database::ID> pageId = webCon.db->getPageId("__front-page");
		if(pageId){
			give404 = false;
			handlePage(reqCon, webCon, "__front-page", *pageId, {});
		}
	}
	else{
        if(uri.size() >= 2 && uri[0] == "__system"){
            if(uri.size() == 4 && uri[1] == "pageFile"){
                std::optional<Database::ID> pageId = webCon.db->getPageId(uri[2]);
                if(pageId){
                    std::string fileName = uri[3];
                    std::optional<Database::ID> fileId = webCon.db->getPageFileId(*pageId, fileName);
                    if(fileId){
                        give404 = false;
                        if(fileName.size() > 4 && fileName.substr(fileName.size() - 4, 4) == ".css"){
                            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
                            << "Content-Type: text/css\r\n\r\n"_AM;
                        }
                        else{
                            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
                            << "Content-Type: image\r\n\r\n"_AM;
                        }
                        webCon.db->downloadPageFile(*fileId, *reqCon.out.getUnsafeRawOutputStream());
                    }
                }
            }
            else if(uri.size() == 3 && uri[1] == "pageVotes"){
				std::optional<Database::ID> pageId = webCon.db->getPageId(uri[2]);
                if(pageId){
					Database::PageRevision revision = webCon.db->getLatestPageRevision(*pageId);
					std::vector<Database::PageVote> votes = webCon.db->getPageVotes(*pageId);
					reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
					<< "Content-Type: text/html; charset=utf-8\r\n\r\n"_AM
					<< "<!DOCTYPE html><html><head><link rel='stylesheet' type='text/css' href='/__static/style.css'>"_AM
					<< "<meta charset='UTF-8'><title>"_AM
					<< revision.title << "</title></head><body><div id='sourceCodeBox'>"_AM
					<< "<h1>Votes on \""_AM << revision.title << "\"</h1>"_AM
					<< "<table><tbody><tr><th>User</th><th>Vote</th></tr>"_AM;
					for(const Database::PageVote& vote : votes){
						Parser::ShownAuthor author = Parser::getShownAuthor(webCon.db.get(), vote.authorId);
						std::string value;
						switch(vote.type){
							default:
								throw std::runtime_error("Attempted to display a vote with an invalid type");
								break;
							case Database::PageVote::Type::Up:
								value = "+1";
								break;
							case Database::PageVote::Type::Down:
								value = "-1";
								break;
						}
						reqCon.out << "<tr><td>"_AM;
						toHtmlShownAuthor(reqCon.out, author);
						reqCon.out << "</td><td>"_AM << value << "</td></tr>"_AM;
					}
					reqCon.out << "</tbody></table></div></body></html>"_AM;
					give404 = false;
                }
            }
        }
        else{
			if(uri.size() > 1 && uri[0] == "forum"){//if they're trying to access the form, we need to translate that into the correct page they actually want
				if(uri[1][0] == 's'){
					uri.insert(uri.begin(), "forum:start");
				}
				else if(uri[1][0] == 'c'){
					uri.insert(uri.begin(), "forum:category");
				}
				else if(uri[1][0] == 't'){
					uri.insert(uri.begin(), "forum:thread");
				}
			}
            std::optional<Database::ID> pageId = webCon.db->getPageId(uri[0]);
            
            std::map<std::string, std::string> parameters;
            
            for(std::size_t i = 1; i < uri.size(); i += 2){
                if(i + 1 < uri.size()){
                    parameters[uri[i]] = uri[i + 1];
                }
                else{
                    parameters[uri[i]] = "";
                }
            }
            
            if(pageId){
                if(handlePage(reqCon, webCon, uri[0], *pageId, parameters)){
                    give404 = false;
                }
            }
        }
	}
	
	if(give404){
		reqCon.out << "HTTP/1.1 404 NOT FOUND\r\n"_AM
		<< "Content-Type: text/html\r\n\r\n"_AM
		<< "<html><body><h1>Page not found</h1></body></html>"_AM;
	}
	
	
	
}

namespace{
	void parsePage(Website::Context& webCon, std::string pageName, const std::map<std::string, std::string>& parameters, const Database::PageRevision& revision, Database::ID pageId, Parser::TokenedPage& pageTokens, Parser::PageTree& pageTree){
		Parser::ParserParameters parserParameters = {};
        parserParameters.database = webCon.db.get();
        parserParameters.page.name = pageName;
        parserParameters.page.tags = webCon.db->getPageTags(pageId);
        parserParameters.page.rating = webCon.db->countPageVotes(pageId);
        parserParameters.urlParameters = parameters;
        
        pageTokens = Parser::tokenizePage(revision.sourceCode, parserParameters);
        pageTree = Parser::makeTreeFromTokenedPage(pageTokens, parserParameters);
	}
}

bool Website::handlePage(Gateway::RequestContext& reqCon, Website::Context& webCon, std::string pageName, Database::ID pageId, std::map<std::string, std::string> parameters){
	
	if(parameters.find("showSource") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
		
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: text/plain; charset=utf-8\r\n\r\n"_AM
		<< allowMarkup(revision.sourceCode);
        return true;
	}
	else if(parameters.find("articleOnly") != parameters.end()){
		Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
        
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: text/html; charset=utf-8\r\n\r\n"_AM
		<< "<!DOCTYPE html><html><head>"_AM
		<< "<meta http-equiv='Content-Security-Policy' content='upgrade-insecure-requests'>"_AM
		<< "<link rel='stylesheet' type='text/css' href='/component:theme/code/1'>"_AM
		<< "<link rel='stylesheet' type='text/css' href='/__static/style.css'>"_AM
		<< "<meta charset='UTF-8'><title>"_AM
		<< revision.title << "</title></head><body><div id='article'>"_AM;
		Parser::convertPageTreeToHtml(reqCon.out, pageTree);
		reqCon.out << "</div></body></html>"_AM;
		return true;
	}
	else if(parameters.find("showTokenJSON") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
		
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: application/json; charset=utf-8\r\n\r\n"_AM;
		
		nlohmann::json tokens = nlohmann::json::array();
		for(const Parser::Token& token : pageTokens.tokens){
			tokens.push_back(Parser::printToken(token));
		}
		reqCon.out << allowMarkup(tokens.dump(4));
        return true;
	}
	else if(parameters.find("showAnnotatedSource") != parameters.end()){
		Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
        
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: text/html; charset=utf-8\r\n\r\n"_AM
		<< "<!DOCTYPE html><html><head><link rel='stylesheet' type='text/css' href='/__static/style.css'>"_AM
		<< "<meta charset='UTF-8'><title>"_AM
		<< revision.title << "</title></head><body><div id='sourceCodeBox'>"_AM;
		
		reqCon.out << "<p>"_AM;
		Parser::convertTokenedPageToHtml(reqCon.out, pageTokens);
		reqCon.out << "</p>"_AM
        << "</div></body></html>"_AM;
        return true;
	}
	else if(parameters.find("showNodeJSON") != parameters.end()){
		Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
		
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: application/json; charset=utf-8\r\n\r\n"_AM;
		
		reqCon.out << allowMarkup(Parser::printNode(pageTree.pageRoot).dump(4));
        return true;
	}
	else if(parameters.find("code") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
        
        std::size_t codeNum;
        try{
            codeNum = std::stoull(parameters.find("code")->second) - 1;
        }
        catch(std::exception& e){
            return false;
        }
        if(pageTree.codeData.size() <= codeNum){
            return false;
        }
        if(pageTree.codeData[codeNum].type == "css"){
            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
            << "Content-Type: text/css; charset=utf-8\r\n\r\n"_AM;
        }
        else{
            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
            << "Content-Type: text/plain; charset=utf-8\r\n\r\n"_AM;
        }
		
		(*reqCon.out.getUnsafeRawOutputStream()) << pageTree.codeData[codeNum].contents;
		return true;
	}
	else if(parameters.find("css") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        Parser::TokenedPage pageTokens;
        Parser::PageTree pageTree;
        parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
        
        std::size_t cssNum;
        try{
            cssNum = std::stoull(parameters.find("css")->second) - 1;
        }
        catch(std::exception& e){
            return false;
        }
        if(pageTree.cssData.size() <= cssNum){
            return false;
        }
        
        reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
        << "Content-Type: text/css; charset=utf-8\r\n\r\n"_AM;
		
		(*reqCon.out.getUnsafeRawOutputStream()) << pageTree.cssData[cssNum].data;
		return true;
	}
	else{//if nothing special is being asked for, then just send a standard formatted article
		return handleFormattedArticle(reqCon, webCon, pageName, pageId, parameters);
	}
}

bool Website::handleFormattedArticle(Gateway::RequestContext& reqCon, Website::Context& webCon, std::string pageName, Database::ID pageId, std::map<std::string, std::string> parameters){
    Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
    
	if(pageName == "forum:start"){
		revision.sourceCode = "[[module ForumStart]]";
		//really strange special case, idk why the page doesn't just have this in it? forum:category and forum:thread do.
	}
    
	Parser::TokenedPage pageTokens;
	Parser::PageTree pageTree;
	parsePage(webCon, pageName, parameters, revision, pageId, pageTokens, pageTree);
    
    //std::cout << Parser::printNode(pageTree.pageRoot).dump(4) << "\n";
    
    reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
    << "Content-Type: text/html; charset=utf-8\r\n\r\n"_AM
    << "<!DOCTYPE html><html><head>"_AM
    << "<meta http-equiv='Content-Security-Policy' content='upgrade-insecure-requests'>"_AM
    << "<link rel='stylesheet' type='text/css' href='/component:theme/code/1'>"_AM
    << "<link rel='stylesheet' type='text/css' href='/__static/style.css'>"_AM
    << "<meta charset='UTF-8'>"_AM;
    if(revision.title == ""){
		reqCon.out << "<title>SCP Conversion Project</title>"_AM;
    }
    else{
		reqCon.out << "<title>"_AM << revision.title << "</title>"_AM;
    }
    for(std::size_t i = 0; i < pageTree.cssData.size(); i++){
        reqCon.out << "<link rel='stylesheet' type='text/css' href='/"_AM << pageName << "/css/"_AM << std::to_string(i) << "'>"_AM;
    }
    reqCon.out << "</head><body>"_AM;
    
    reqCon.out << "<div id='fullPageContainer'>"_AM
    << "<div id='pageHeaderBackground'>"_AM
    << "<div id='pageHeader'>"_AM
    << "<img id='headerImage' src='/__static/logo.png'>"_AM
    << "<h1 id='headerTitle'><a href='/'>SCP Conversion Project</a></h1>"_AM
    << "<h2 id='headerSubtitle'>Converting and Archiving the SCP Wiki</h2>"_AM
    << "</div></div>"_AM
    << "<div id='pageBody'>"_AM;
    {//side bar content
        reqCon.out << "<div id='sideBar'><div id='side-bar'>"_AM;
        std::string sideBarPageName = "nav:side";
        
        std::optional<Database::ID> sideBarId = webCon.db->getPageId(sideBarPageName);
        
        if(sideBarId){
            Database::PageRevision sideBarRevision = webCon.db->getLatestPageRevision(*sideBarId);
            
            Parser::ParserParameters navSideParameters = {};
			navSideParameters.database = webCon.db.get();
			navSideParameters.page.name = sideBarPageName;
			navSideParameters.page.tags = webCon.db->getPageTags(*sideBarId);
            
            Parser::TokenedPage sideBarTokens = Parser::tokenizePage(sideBarRevision.sourceCode, navSideParameters);
            Parser::PageTree sideBarTree = Parser::makeTreeFromTokenedPage(sideBarTokens, navSideParameters);
            
            Parser::convertPageTreeToHtml(reqCon.out, sideBarTree);
        }
        else{
            reqCon.out << "Side bar content unavailable";
        }
        reqCon.out << "</div></div>"_AM;
    }
    {//the actual article html
        reqCon.out << "<div id='article'>"_AM
        << "<div id='articleTitle'>"_AM << revision.title << "</div>"_AM;
        Parser::convertPageTreeToHtml(reqCon.out, pageTree);
        reqCon.out << "</div>"_AM;
    }
    {//footer
        reqCon.out << "<div id='articleFooter'>"_AM;
        {
			std::optional<std::string> discussion = webCon.db->getPageDiscussion(pageId);
			if(discussion){
				reqCon.out << "<a class='item' href='/forum/t-"_AM << discussion.value() << "'>Discuss</a>"_AM;
			}
        }
        {
			std::int64_t rating = webCon.db->countPageVotes(pageId);
			std::string ratingStr;
			if(rating > 0){
				ratingStr = "+" + std::to_string(rating);
			}
			else{
				ratingStr = std::to_string(rating);
			}
			reqCon.out 
			<< "<a class='item' href='/__system/pageVotes/"_AM << pageName << "'>Ratings("_AM << ratingStr << ")</a>"_AM;
        }
        reqCon.out
        << "<a class='item' href='/"_AM << pageName << "/showAnnotatedSource'>Annotated Source</a>"_AM
        << "<a class='item' href='/"_AM << pageName << "/showSource'>Raw Source</a>"_AM
        << "<a class='item' href='/"_AM << pageName << "/showTokenJSON'>Token JSON</a>"_AM
        << "<a class='item' href='/"_AM << pageName << "/showNodeJSON'>Node JSON</a>"_AM
        << "</div>"_AM;
    }
    reqCon.out << "</div>"_AM //id='pageBody'
    << "<div id='pageFooterBackground'><div id='pageFooter'>"_AM
    << "<div id='pageFooterPoweredBy'>"_AM
    << "Powered by the <a href='https://github.com/danielbatterystapler/SCP_Conversion_Project'>SCP Conversion Project</a><br>"_AM
    << "</div><div id='pageFooterCopyright'>"_AM
    << "Unless otherwise stated, the content of this page is licensed under <a href='https://creativecommons.org/licenses/by-sa/3.0/'>Creative Commons Attribution-ShareAlike 3.0 License</a>"_AM
    << "</div>"_AM
    << "</div></div>"_AM
    << "</div></body></html>"_AM;
    
    return true;
}








