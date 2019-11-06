#include "Website.hpp"

#include "../Config.hpp"

#include <sstream>

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
			
			if(uri.size() == 1){
				std::unique_ptr<Database> db = Database::connectToMongoDatabase(Config::getProductionDatabaseName());
				
				std::optional<Database::ID> pageId = db->getPageId(uri[0]);
				if(pageId){
					Database::PageRevision revision = db->getLatestPageRevision(*pageId);
					
					context.out << "HTTP/1.1 200 OK\r\n"_AM
					<< "Content-Type: text/html\r\n\r\n"_AM;
					
					for(char c : revision.sourceCode){
						if(c == '\n'){
							context.out << "<br>"_AM;
						}
						else{
							context.out << (c + std::string());
						}
					}
					
					threadContext.finishRequest(std::move(context));
				}
				else{
					context.out << "HTTP/1.1 404 NOT FOUND\r\n"_AM
					<< "Content-Type: text/html\r\n\r\n"_AM
					<< "<html><body><h1>Page not found</h1></body></html>"_AM;
					
					threadContext.finishRequest(std::move(context));
				}
			}
			else{
				context.out << "HTTP/1.1 200 OK\r\n"_AM
				<< "Content-Type: text/html\r\n\r\n"_AM
				<< "<html><body><h1>Hello from the SCP Conversion Project!</h1></body></html>"_AM;
				
				threadContext.finishRequest(std::move(context));
			}
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