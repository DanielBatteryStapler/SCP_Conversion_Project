#include "Gateway.hpp"

#include <thread>

#include <sys/socket.h>
#include <csignal>

namespace{
	constexpr unsigned int numberOfGatewayThreads = 16;
}

void Gateway::setup(std::string domainName){
	domain = domainName;
}

void Gateway::run(const char* socket, std::function<void(Gateway::ThreadContext)> threadFunc){
	shutdownFlag = false;
	fcgiListenSocket = FCGX_OpenSocket(socket, 500);
	FCGX_Init();
	
	auto signalHandler = [](int signum){
		std::cout << "\nSIGNAL INPUT:Start Shutdown Procedure\n";
		Gateway::shutdown();
	};
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	std::vector<std::thread> threadPool;
	for(int i = 0; i < numberOfGatewayThreads; i++){
        ThreadContext threadContext;
        
        threadContext.domainName = domain;
        threadContext.requestNeedsFinish = false;
        
        FCGX_InitRequest(&threadContext.fcgiRequest, fcgiListenSocket, 0);
        
        threadPool.push_back(std::thread(threadFunc, threadContext));
	}
	
	for(int i = 0; i < threadPool.size(); i++){
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
		requestNeedsFinish = true;
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



