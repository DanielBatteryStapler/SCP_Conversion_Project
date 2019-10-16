#include <iostream>

#include "Tests/Tests.hpp"

#include <mongocxx/instance.hpp>


int main(int argc, char** argv){
	
	mongocxx::instance instance{};//this needs to exist for the entire program so mongodb works
	
	if(argc == 2 && std::string(argv[1]) == "--runTests"){
		Tests::runAllTests();
	}
	else{
		std::cout << "Invalid/No Options Specified\n";
	}
	
	return 0;
}
