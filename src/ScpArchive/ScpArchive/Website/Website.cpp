#include "Website.hpp"

#include "../Config.hpp"

#include <sstream>

#include "../Parser/HTMLConverter.hpp"

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
		websiteContext.db = Database::connectToMongoDatabase(Config::getProductionDatabaseName());
		
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
		std::optional<Database::ID> pageId = webCon.db->getPageId("main");
		if(pageId){
			give404 = false;
			handlePage(reqCon, webCon, "main", *pageId, {});
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
                                std::cout << "IS CSS\n";
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
        }
        else{
            std::optional<Database::ID> pageId = webCon.db->getPageId(uri[0]);
            
            std::map<std::string, std::string> parameters;
            
            for(int i = 1; i < uri.size(); i += 2){
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

bool Website::handlePage(Gateway::RequestContext& reqCon, Website::Context& webCon, std::string pageName, Database::ID pageId, std::map<std::string, std::string> parameters){
	if(parameters.find("showAnnotatedSource") != parameters.end()){
		Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        
        Parser::ParserParameters parserParameters = {};
        parserParameters.database = webCon.db.get();
        parserParameters.page.name = pageName;
        parserParameters.page.tags = webCon.db->getPageTags(pageId);
        
        Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode, parserParameters);
		
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: text/html\r\n\r\n"_AM
		<< "<!DOCTYPE html><html><head><link rel='stylesheet' type='text/css' href='/static/style.css'><meta charset='UTF-8'><title>"_AM
		<< revision.title << "</title></head><body>"_AM;
		
		reqCon.out << "<p>"_AM;
		Parser::convertTokenedPageToHtml(reqCon.out, pageTokens);
		reqCon.out << "</p>"_AM
        << "</body></html>"_AM;
        return true;
	}
	else if(parameters.find("showSource") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
		
		reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
		<< "Content-Type: text/html\r\n\r\n"_AM
		<< "<!DOCTYPE html><html><head><link rel='stylesheet' type='text/css' href='/static/style.css'><meta charset='UTF-8'><title>"_AM
		<< revision.title << "</title></head><body>"_AM;
		
		reqCon.out << "<p>"_AM;
		for(char c : revision.sourceCode){
			if(c == '\n'){
				reqCon.out << "<br />\n"_AM;
			}
			else if(c == ' '){
				reqCon.out << "&ensp;"_AM;
			}
			else{
				reqCon.out << c;
			}
		}
		reqCon.out << "</p>"_AM
        << "</body></html>"_AM;
        return true;
	}
	else if(parameters.find("code") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        
        Parser::ParserParameters parserParameters = {};
        parserParameters.database = webCon.db.get();
        parserParameters.page.name = pageName;
        parserParameters.page.tags = webCon.db->getPageTags(pageId);
        
        Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode, parserParameters);
        Parser::PageTree pageTree = Parser::makeTreeFromTokenedPage(pageTokens, parserParameters);
        
        int codeNum;
        try{
            codeNum = std::stoi(parameters.find("code")->second) - 1;
        }
        catch(std::exception& e){
            return false;
        }
        if(codeNum < 0 || pageTree.codeData.size() <= codeNum){
            return false;
        }
        if(pageTree.codeData[codeNum].type == "css"){
            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
            << "Content-Type: text/css\r\n\r\n"_AM;
        }
        else{
            reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
            << "Content-Type: text/plain\r\n\r\n"_AM;
        }
		
		(*reqCon.out.getUnsafeRawOutputStream()) << pageTree.codeData[codeNum].contents;
		return true;
	}
	else if(parameters.find("css") != parameters.end()){
        Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
        
        Parser::ParserParameters parserParameters = {};
        parserParameters.database = webCon.db.get();
        parserParameters.page.name = pageName;
        parserParameters.page.tags = webCon.db->getPageTags(pageId);
        
        Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode, parserParameters);
        Parser::PageTree pageTree = Parser::makeTreeFromTokenedPage(pageTokens, parserParameters);
        
        int cssNum;
        try{
            cssNum = std::stoi(parameters.find("css")->second) - 1;
        }
        catch(std::exception& e){
            return false;
        }
        if(cssNum < 0 || pageTree.cssData.size() <= cssNum){
            return false;
        }
        
        reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
        << "Content-Type: text/css\r\n\r\n"_AM;
		
		(*reqCon.out.getUnsafeRawOutputStream()) << pageTree.cssData[cssNum].data;
		return true;
	}
	else{//if nothing special is being asked for, then just send a standard formatted article
		return handleFormattedArticle(reqCon, webCon, pageName, pageId, parameters);
	}
}

bool Website::handleFormattedArticle(Gateway::RequestContext& reqCon, Website::Context& webCon, std::string pageName, Database::ID pageId, std::map<std::string, std::string> parameters){
    
    Database::PageRevision revision = webCon.db->getLatestPageRevision(pageId);
    
    Parser::ParserParameters parserParameters = {};
    parserParameters.database = webCon.db.get();
    parserParameters.page.name = pageName;
    parserParameters.page.tags = webCon.db->getPageTags(pageId);
    
    Parser::TokenedPage pageTokens = Parser::tokenizePage(revision.sourceCode, parserParameters);
    Parser::PageTree pageTree = Parser::makeTreeFromTokenedPage(pageTokens, parserParameters);
    
    reqCon.out << "HTTP/1.1 200 OK\r\n"_AM
    << "Content-Type: text/html\r\n\r\n"_AM
    << "<!DOCTYPE html><html><head>"_AM
    << "<meta http-equiv='Content-Security-Policy' content='upgrade-insecure-requests'>"_AM
    << "<link rel='stylesheet' type='text/css' href='/component:theme/code/1'>"_AM
    << "<link rel='stylesheet' type='text/css' href='/static/style.css'>"_AM
    << "<meta charset='UTF-8'>"_AM
    << "<title>"_AM << revision.title << "</title>"_AM;
    for(std::size_t i = 0; i < pageTree.cssData.size(); i++){
        reqCon.out << "<link rel='stylesheet' type='text/css' href='/"_AM << pageName << "/css/"_AM << std::to_string(i) << "'>"_AM;
    }
    reqCon.out << "</head><body>"_AM;
    
    reqCon.out << "<div id='fullPageContainer'>"_AM
    << "<div id='pageHeaderBackground'>"_AM
    << "<div id='pageHeader'>"_AM
    << "<img id='headerImage' src='/static/logo.png'>"_AM
    << "<h1 id='headerTitle'><a href='/'>SCP Conversion Project</a></h1>"_AM
    << "<h2 id='headerSubtitle'>Converting and Archiving the SCP Wiki</h2>"_AM
    << "</div></div>"_AM
    << "<div id='pageBody'>"_AM
    << "<div id='sideBar'><div id='side-bar'>"_AM;
    {//side bar content
        std::string sideBarPageName = "nav:side";
        
        std::optional<Database::ID> sideBarId = webCon.db->getPageId(sideBarPageName);
        
        if(sideBarId){
            Database::PageRevision sideBarRevision = webCon.db->getLatestPageRevision(*sideBarId);
            
            Parser::TokenedPage sideBarTokens = Parser::tokenizePage(sideBarRevision.sourceCode, parserParameters);
            Parser::PageTree sideBarTree = Parser::makeTreeFromTokenedPage(sideBarTokens, parserParameters);
            
            Parser::convertPageTreeToHtml(reqCon.out, sideBarTree);
        }
        else{
            reqCon.out << "Side bar content unavailable";
        }
        
    }
    reqCon.out << "</div></div>"_AM;
    //the actual article html
    reqCon.out << "<div id='article'>"_AM
    << "<div id='articleTitle'>"_AM << revision.title << "</div>"_AM;
    Parser::convertPageTreeToHtml(reqCon.out, pageTree);
    reqCon.out << "</div>"_AM
    << "</div></div>"_AM
    << "</body></html>"_AM;
    
    return true;
}








