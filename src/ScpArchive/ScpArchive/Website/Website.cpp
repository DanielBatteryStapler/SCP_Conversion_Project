#include "Website.hpp"

#include "../Config.hpp"

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
			std::cout << "Got Request!\n";
			
			context.out << "HTTP/1.1 200 OK\r\n"_AM
			<< "Content-Type: text/html\r\n\r\n"_AM
			<< "<html><body><h1>Hello from the SCP Conversion Project!</h1></body></html>"_AM;
			
			threadContext.finishRequest(std::move(context));
		}
	}
}
