#ifndef WEBSITE_HPP
#define WEBSITE_HPP

#include "Gateway.hpp"
#include "../Database/Database.hpp"

class Website{
	public:
		Website() = delete;
		
		static void run();
		
	private:
		static void threadProcess(Gateway::ThreadContext threadContext);
		static std::vector<std::string> splitUri(std::string uri);
};

#endif // WEBSITE_HPP
