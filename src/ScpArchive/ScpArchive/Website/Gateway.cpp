#include "Gateway.hpp"

#include <thread>

#include <sys/socket.h>
#include <csignal>

#include "../Config.hpp"

void Gateway::setup(std::string domainName){
	domain = domainName;
}

void Gateway::run(std::string socket, std::function<void(Gateway::ThreadContext)> threadFunc){
	shutdownFlag = false;
	fcgiListenSocket = FCGX_OpenSocket(socket.c_str(), 500);
	FCGX_Init();
	
	auto signalHandler = [](int signum){
		std::cout << "\nSIGNAL INPUT:Start Shutdown Procedure\n";
		Gateway::shutdown();
	};
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	unsigned int numberOfGatewayThreads = Config::getNumberOfThreadsForGateway();
	
	std::vector<std::thread> threadPool;
	for(unsigned int i = 0; i < numberOfGatewayThreads; i++){
        ThreadContext threadContext;
        threadContext.threadIndex = i;
        threadContext.domainName = domain;
        FCGX_InitRequest(&threadContext.fcgiRequest, fcgiListenSocket, 0);
        
        threadPool.push_back(std::thread(threadFunc, threadContext));
	}
	
	for(unsigned int i = 0; i < numberOfGatewayThreads; i++){
        threadPool[i].join();
	}
}

void Gateway::shutdown(){
	shutdownFlag = true;
	FCGX_ShutdownPending();
	::shutdown(fcgiListenSocket, SHUT_RDWR);
}



Gateway::RequestContext Gateway::ThreadContext::getNextRequest(){
	if(FCGX_Accept_r(&fcgiRequest) == 0){
		RequestContext output;
		
		output.shouldShutdown = false;
		output.fcgiOutBuffer = std::make_unique<fcgi_streambuf>(fcgiRequest.out);
		output.rawOutputStream = std::make_unique<std::ostream>(output.fcgiOutBuffer.get());
		
		output.fcgiInputWrapper.fcgiRequest = &fcgiRequest;
		output.cgi = std::make_unique<cgicc::Cgicc>(&output.fcgiInputWrapper);
		output.env = &output.cgi->getEnvironment();
		
		output.out = MarkupOutStream(output.rawOutputStream.get());
		
		return output;
	}
	else{
		RequestContext output;
		output.shouldShutdown = true;
		return output;
	}
}

void Gateway::ThreadContext::finishRequest(Gateway::RequestContext requestContext){
	FCGX_Finish_r(&fcgiRequest);
}

std::size_t Gateway::FcgiInputWrapper::read(char* data, std::size_t length){
	return FCGX_GetStr(data, length, fcgiRequest->in);
}

std::string Gateway::FcgiInputWrapper::getenv(const char* varName){
	const char* temp = FCGX_GetParam(varName, fcgiRequest->envp);
	if(temp == nullptr){
		return std::string();
	}
	else{
		return std::string(temp);
	}
}

std::string Gateway::RequestContext::getUriString(){
	return fcgiInputWrapper.getenv("DOCUMENT_URI");
}


