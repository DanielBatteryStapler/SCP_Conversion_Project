
#include <Magick++.h>

#include "Ncurses.hpp"

#include <csignal>
#include <unistd.h>
#include <iostream>

static void handler(int signum){
	printf("\n");
	exit(0);
}

int main(){
	Magick::InitializeMagick(nullptr);
	
	struct sigaction sa = {};
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	
	sigaction(SIGINT, &sa, NULL);
	
	try{
		runNcursesTerminal();
	}
	catch(std::exception& e){
		std::cout << "exception: " << e.what() << "\n";
		while(true){
			
		}
	}
	
	return 0;
}
