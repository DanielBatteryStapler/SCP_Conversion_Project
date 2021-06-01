#include "Ncurses.hpp"

#include <ncurses.h>
#include <termios.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Database.hpp"

#include "Cli.hpp"
#include "layout/Layout.hpp"

namespace{
	//store original termios so we can just reset to it to disable raw mode
	struct termios tty_attr_original;

	void tty_disable_raw_mode(void){
		tcsetattr(0, TCSANOW, &tty_attr_original);
	}

	void tty_raw_mode(void){
		struct termios tty_attr;

		tcgetattr(0,&tty_attr);

		tty_attr.c_lflag &= (~(ICANON|ECHO));
		tty_attr.c_cc[VTIME] = 0;
		tty_attr.c_cc[VMIN] = 1;

		tcsetattr(0,TCSANOW,&tty_attr);
	}
}

namespace{
	bool isNcursesMode = false;

	std::vector<GlyphAttributes::Color> colors;

	int getColorPair(int red, int green, int blue){
		if(has_colors() == false || can_change_color() == false){
			throw std::runtime_error{"cannot set color"};
		}

		GlyphAttributes::Color color{red, green, blue};

		for(int i = 0; i < colors.size(); i++){
			if(colors[i] == color){
				//std::cout << "color:" << i << "\n";
				return i + 1;
			}
		}

		int colorIndex = colors.size() + 8;
		int pairIndex = colors.size() + 1;

		if(COLORS <= colorIndex){
			throw std::runtime_error{"not enough colors"};
		}
		if(COLOR_PAIRS <= pairIndex){
			throw std::runtime_error{"not enough pairs"};
		}

		//std::cout << "making color: " << colorIndex << ", " << pairIndex << ", " << static_cast<int>(red / 255.0 * 1000) << ", " << green << ", " << static_cast<int>(green / 255.0 * 1000) << ", " << static_cast<int>(blue / 255.0 * 1000) << "\n";

		if(init_color(colorIndex, static_cast<int>(red / 255.0 * 1000), static_cast<int>(green / 255.0 * 1000), static_cast<int>(blue / 255.0 * 1000)) != 0){
			throw std::runtime_error{"error on init_extended_color"};
		}
		/*
		int r;
		int g;
		int b;
		extended_color_content(colorIndex, &r, &g, &b);
		*/

		if(init_pair(pairIndex, colorIndex, COLOR_BLACK) != 0){
			throw std::runtime_error{"error on init_extended_pair"};
		}
		/*
		int fg;
		int bg;
		extended_pair_content(pairIndex, &fg, &bg);
		if(fg != colorIndex){
			throw std::runtime_error{"check fail"};
		}
		if(static_cast<int>(red / 255.0 * 1000) != r){
			throw std::runtime_error{"check fail"};
		}
		if(static_cast<int>(green / 255.0 * 1000) != g){
			throw std::runtime_error{"check fail"};
		}
		if(static_cast<int>(blue / 255.0 * 1000) != b){
			throw std::runtime_error{"check fail"};
		}
		*/

		colors.push_back(color);
		return pairIndex;
	}
}

void runNcursesTerminal(){
	const std::function<void(nlohmann::json)> doOutputCommands = [&doOutputCommands](nlohmann::json outputs){
		for(nlohmann::json& output : outputs){
			std::string type = output["type"].get<std::string>();

			if(type == "startFullscreen"){
				setlocale(LC_ALL, "");
				initscr();
				start_color();
				cbreak();
				noecho();
				isNcursesMode = true;
			}
			else if(type == "stopFullscreen"){
				endwin();
				isNcursesMode = false;
			}
			else if(type == "fullscreenPrint"){

				colors.clear();
				
				wmove(stdscr, 0, 0);
				for(nlohmann::json& grid : output["grid"]){
					if(grid["bold"].get<bool>()){
						attron(A_BOLD);
					}

					if(grid["italic"].get<bool>()){
						attron(A_ITALIC);
					}

					if(grid["underline"].get<bool>()){
						attron(A_UNDERLINE);
					}

					if(grid["colorSet"].get<bool>()){
						attron(COLOR_PAIR(getColorPair(grid["red"].get<int>(), grid["green"].get<int>(), grid["blue"].get<int>())));
					}

					waddstr(stdscr, grid["glyphs"].get<std::string>().c_str());

					if(grid["colorSet"].get<bool>()){
						attroff(COLOR_PAIR(getColorPair(grid["red"].get<int>(), grid["green"].get<int>(), grid["blue"].get<int>())));
					}

					if(grid["bold"].get<bool>()){
						attroff(A_BOLD);
					}

					if(grid["italic"].get<bool>()){
						attroff(A_ITALIC);
					}

					if(grid["underline"].get<bool>()){
						attroff(A_UNDERLINE);
					}
				}
				int cursorX = output["cursorX"].get<int>();
				int cursorY = output["cursorY"].get<int>();
				if(cursorX == -1){
					curs_set(0);
				}
				else{
					wmove(stdscr, cursorY, cursorX);
					curs_set(1);
				}

			}
			else if(type == "loadArticle"){
				nlohmann::json jsonInput;
				jsonInput["type"] = "loadedArticle";
				jsonInput["articleJson"] = Database::getPage(output["articleUrl"].get<std::string>());
				doOutputCommands(Cli::processCliProxy(jsonInput));
			}
			else if(type == "print"){
				std::cout << output["text"].get<std::string>() << std::flush;
			}
			else{
				throw std::runtime_error("invalid output type");
			}
		}
		if(isNcursesMode){
			wrefresh(stdscr);
		}
	};

	tcgetattr(0,&tty_attr_original);
	atexit(tty_disable_raw_mode);
	tty_raw_mode();

	{
		nlohmann::json jsonInput;
		jsonInput["type"] = "startup";
		doOutputCommands(Cli::processCliProxy(jsonInput));
	}

	while(true){

		if(isNcursesMode){
			int x;
			int y;
			getmaxyx(stdscr, y, x);
			nlohmann::json jsonInput;
			jsonInput["type"] = "size";
			jsonInput["width"] = x;
			jsonInput["height"] = y;
			doOutputCommands(Cli::processCliProxy(jsonInput));
		}

		std::string input;

		char ch;
		if(isNcursesMode){
			ch = wgetch(stdscr);
		}
		else{
			read(0, &ch, 1);
		}

		if(ch == 27){
			//if we got an escape sequence, lets see what it is

			char ch1;
			char ch2;
			char ch3 = 0;

			if(isNcursesMode){
				ch1 = wgetch(stdscr);
				ch2 = wgetch(stdscr);
			}
			else{
				read(0, &ch1, 1);
				read(0, &ch2, 1);
			}

			if(ch1 == '[' && ch2 == 'A'){
				input = "up";
			}
			else if(ch1 == '[' && ch2 == 'B'){
				input = "down";
			}
			else if(ch1 == '[' && ch2 == 'C'){
				input = "right";
			}
			else if(ch1 == '[' && ch2 == 'D'){
				input = "left";
			}
		}
		else{
			input = ch;
		}

		if(input != ""){
			nlohmann::json jsonInput;
			jsonInput["type"] = "input";
			jsonInput["input"] = input;
			doOutputCommands(Cli::processCliProxy(jsonInput));
		}
	}

	tty_disable_raw_mode();
}