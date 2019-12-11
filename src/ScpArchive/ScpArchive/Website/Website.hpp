#ifndef WEBSITE_HPP
#define WEBSITE_HPP

#include "Gateway.hpp"
#include "../Database/Database.hpp"

class Website{
	public:
		struct Context{
			std::unique_ptr<Database> db;
		};
		
		Website() = delete;
		
		static void run();
		
	private:
		static void threadProcess(Gateway::ThreadContext threadContext);
		static std::vector<std::string> splitUri(std::string uri);
		static void handleUri(Gateway::RequestContext& reqCon, Website::Context& webCon, std::vector<std::string> uri);
		static void handlePage(Gateway::RequestContext& reqCon, Website::Context& webCon, Database::ID pageId, std::map<std::string, std::string> parameters);
};

#endif // WEBSITE_HPP
