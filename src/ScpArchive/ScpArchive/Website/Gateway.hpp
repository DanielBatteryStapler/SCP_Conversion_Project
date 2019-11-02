#ifndef GATEWAY_HPP
#define GATEWAY_HPP

#include <string>
#include <atomic>
#include <functional>
#include <memory>

#include <fcgio.h>

#include <cgicc/Cgicc.h>
#include <cgicc/CgiDefs.h>
#include <cgicc/HTTPHeader.h>
#include <cgicc/HTTPStatusHeader.h>
#include <cgicc/CgiEnvironment.h>
#include <cgicc/CgiUtils.h>

#include "../Parser/MarkupOutStream.hpp"

class Gateway{
	private:
		struct FcgiInputWrapper : public cgicc::CgiInput{
			FCGX_Request* fcgiRequest;
			virtual std::size_t read(char* data, std::size_t length);
			virtual std::string getenv(const char* varName);
		};
	public:
		struct RequestContext{
			friend class Gateway;
			
			public:
				bool shouldShutdown;
				std::unique_ptr<cgicc::Cgicc> cgi;
				const cgicc::CgiEnvironment* env;
				MarkupOutStream out;
			
			private:
				FcgiInputWrapper fcgiInputWrapper;
				std::unique_ptr<fcgi_streambuf> fcgiOutBuffer;
				std::unique_ptr<std::ostream> rawOutputStream;
		};
		
		struct ThreadContext{
			friend class Gateway;
			
            public:
                Gateway::RequestContext getNextRequest();
                void finishRequest(Gateway::RequestContext request);
				
                std::string domainName;
                unsigned int threadIndex;
            private:
                FCGX_Request fcgiRequest;
		};
		
		Gateway() = delete;
		
		static void setup(std::string domainName);
		
		static void run(std::string socket, std::function<void(Gateway::ThreadContext)> threadFunc);
		static void shutdown();
		
	private:
		static inline std::atomic<bool> shutdownFlag;
		static inline std::string domain;
        
        static inline int fcgiListenSocket;
};

#endif // GATEWAY_HPP
