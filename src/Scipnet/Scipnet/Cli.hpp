#ifndef CLI_HPP
#define CLI_HPP

#include "nlohmann/json.hpp"

namespace Cli{
	nlohmann::json processCliProxy(nlohmann::json input);
	nlohmann::json processCli(nlohmann::json input);
}

#endif // CLI_HPP
