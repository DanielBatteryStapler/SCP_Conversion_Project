
#include <stdio.h>

#include "Cli.hpp"
#include "Helpers.hpp"

#include <emscripten/bind.h>
using namespace emscripten;

extern "C" int main(int argc, char** argv){
	printf("Hello from SCiPNET wasm!\n");
	return 0;
}

std::string processCliString(std::string jsonInput){
	return Cli::processCli(nlohmann::json::parse(jsonInput)).dump();
}

EMSCRIPTEN_BINDINGS(scipnet_module){
	function("processCliString", &processCliString);
	function("redirectLink", &redirectLink);
}