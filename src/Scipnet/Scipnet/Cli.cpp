#include "Cli.hpp"

#include <sstream>

#include <string>
#include "layout/Layout.hpp"
#include "Converter.hpp"

namespace{
	bool noCaseEquals(std::string a, std::string b){
		std::transform(a.begin(), a.end(), a.begin(), ::tolower);
		std::transform(b.begin(), b.end(), b.begin(), ::tolower);
		return a == b;
	}
}

namespace Cli{

	nlohmann::json processCliProxy(nlohmann::json input){

		//std::ofstream{"test.txt", std::ios_base::app} << "<<<" << input["type"].get<std::string>() << "\n";

		nlohmann::json output = processCli(input);

		//for(nlohmann::json i : output){
		//	std::ofstream{"test.txt", std::ios_base::app} << ">>>" << i["type"].get<std::string>() << "\n";
		//}
		//std::ofstream{"test.txt", std::ios_base::app} << "\n";

		return output;
	}

	namespace{
		int currentWidth = 80;
		int currentHeight = 80;

		std::string currentLine;
		std::size_t currentPos = 0;
		
		struct PageEntry{
			std::shared_ptr<Element> layout;
			OutputBuffer layoutOutput;
			int currentYPos = 0;
			int currentXPos = 0;
		};
		std::vector<PageEntry> pageEntries;
		int currentPage = -1;
		
		int currentClickable = -1;
	}

	void printText(nlohmann::json& outputs, std::string text){
		nlohmann::json output;
		output["type"] = "print";
		output["text"] = text;
		outputs.push_back(output);
	}

	void handleCommand(nlohmann::json& outputs, std::string command);

	nlohmann::json processCli(nlohmann::json input){
		nlohmann::json outputs = nlohmann::json::array();

		const auto print = [&](std::string text){
			printText(outputs, text);
		};

		std::string type = input["type"].get<std::string>();

		if(type == "size"){
			if(currentWidth != input["width"].get<int>() || currentHeight != input["height"].get<int>()){
				currentWidth = input["width"].get<int>();
				currentHeight = input["height"].get<int>();
				
				if(currentPage != -1){
					PageEntry& entry = pageEntries[currentPage];
					
					entry.layout->widthLimit = currentWidth;
					entry.layoutOutput = drawLayout(entry.layout);
					outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
				}
			}
		}
		else if(currentPage == -1){

			if(type == "startup"){
				print(
					"WELCOME TO SCiPNET DIRECT ACCESS TERMINAL. PLEASE ENTER COMMAND\n"
					"\n"
					"SCiPNET> ");
			}
			else if(type == "input"){
				std::string text = input["input"].get<std::string>();
				if(text == "up"){

				}
				else if(text == "down"){

				}
				else if(text == "right"){
					if(currentPos != currentLine.size()){
						print(std::string{currentLine[currentPos]});
						currentPos++;
					}
				}
				else if(text == "left"){
					if(currentPos != 0){
						print("\b");
						currentPos--;
					}
				}
				else if(text == "\b" || (text.size() == 1 && text[0] == 127)){
					if(currentPos != 0){
						currentLine.erase(currentPos - 1, 1);
						currentPos--;
						print("\b");
						print(currentLine.substr(currentPos));
						print(" ");
						for(std::size_t i = currentPos; i < currentLine.size(); i++){
							print("\b");
						}
						print("\b");
					}
				}
				else if(text == "\n"){
					print("\n");
					handleCommand(outputs, currentLine);
					currentLine = "";
					currentPos = 0;
				}
				else{
					currentLine.insert(currentPos, text);
					print(currentLine.substr(currentPos));
					currentPos++;
					for(std::size_t i = currentPos; i < currentLine.size(); i++){
						print("\b");
					}
				}
			}
			else if(type == "loadedArticle"){
				PageEntry entry;
				entry.layout = Converter::makeLayoutFromPageJson(input["articleJson"]);
				entry.layout->widthLimit = currentWidth;
				entry.layoutOutput = drawLayout(entry.layout);
				{
					nlohmann::json output;
					output["type"] = "startFullscreen";
					outputs.push_back(output);
				}
				outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
				while(pageEntries.size() != currentPage + 1){
					pageEntries.pop_back();
				}
				currentPage = pageEntries.size();
				pageEntries.push_back(entry);
				currentClickable = -1;
			}
			else{
				throw std::runtime_error("invalid input type");
			}
		}
		else{
			if(type == "input"){
				std::string text = input["input"].get<std::string>();
				if(text == "up"){
					PageEntry& entry = pageEntries[currentPage];
					if(entry.currentYPos > 0){
						entry.currentYPos--;
						currentClickable = -1;
						outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
					}
				}
				else if(text == "down"){
					PageEntry& entry = pageEntries[currentPage];
					if(entry.currentYPos < (entry.layoutOutput.height - 2)){
						entry.currentYPos++;
						currentClickable = -1;
						outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
					}
				}
				else if(text == "right" || text == "left" || text == "\n"){
					PageEntry& entry = pageEntries[currentPage];
					std::vector<std::shared_ptr<Element>> clickables = getClickableElements(entry.layout);
					std::vector<std::shared_ptr<Element>> clickablesInView;

					for(const auto& i : clickables){
						if(i->bounds.x >= entry.currentXPos && i->bounds.x < entry.currentXPos + currentWidth){
							if(i->bounds.y >= entry.currentYPos && i->bounds.y < entry.currentYPos + currentHeight){
								clickablesInView.push_back(i);
							}
						}
					}

					if(text == "right" || text == "left"){
						if(text == "left"){
							if(currentClickable > -1){
								currentClickable--;
							}
						}
						else if(text == "right"){
							if(currentClickable + 1 < clickablesInView.size()){
								currentClickable++;
							}
						}
						int cursorXPos = -1;
						int cursorYPos = -1;
						if(currentClickable != -1){
							cursorXPos = clickablesInView[currentClickable]->bounds.x;
							cursorYPos = clickablesInView[currentClickable]->bounds.y;
						}
						outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight, cursorXPos, cursorYPos));
					}
					else if(text == "\n"){
						if(currentClickable != -1){
							nlohmann::json clickOrder = clickablesInView[currentClickable]->click();

							//re-render in case performing the click changed the layout(e.g. collapsibles)
							entry.layout->widthLimit = currentWidth;
							entry.layoutOutput = drawLayout(entry.layout);
							currentClickable = -1;
							outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
							
							if(clickOrder != nullptr){
								std::string orderType = clickOrder["type"].get<std::string>();
								if(orderType == "link"){
									nlohmann::json output;
									output["type"] = "loadArticle";
									output["articleUrl"] = clickOrder["url"].get<std::string>();
									outputs.push_back(output);
								}
								else{
									throw std::runtime_error("invalid click order type");
								}
							}
						}
					}
				}
				else if(text == "1" || text == "2"){
					currentClickable = -1;
					if(text == "1"){
						if(currentPage > 0){
							currentPage--;
						}
					}
					else if(text == "2"){
						if(currentPage < pageEntries.size() - 1){
							currentPage++;
						}
					}
					PageEntry& entry = pageEntries[currentPage];
					outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
				}
				else if(text == "q"){
					pageEntries.clear();
					currentPage = -1;
					{
						nlohmann::json output;
						output["type"] = "stopFullscreen";
						outputs.push_back(output);
					}
					print("SCiPNET> ");
				}
				else{
					//do nothing
				}
			}
			else if(type == "loadedArticle"){
				PageEntry entry;
				entry.layout = Converter::makeLayoutFromPageJson(input["articleJson"]);
				entry.layout->widthLimit = currentWidth;
				entry.layoutOutput = drawLayout(entry.layout);
				outputs.push_back(entry.layoutOutput.makeJsonOutput(entry.currentXPos, entry.currentYPos, currentWidth, currentHeight));
				while(pageEntries.size() != currentPage + 1){
					pageEntries.pop_back();
				}
				currentPage = pageEntries.size();
				pageEntries.push_back(entry);
				currentClickable = -1;
			}
			else{
				throw std::runtime_error("invalid input type");
			}
		}

		return outputs;
	}
	
	namespace{
		std::vector<std::string> splitCommandString(std::string command){
			std::vector<std::string> commandParameters;
			{
				std::stringstream ss{command};
				std::string temp;
				while(std::getline(ss, temp, ' ')){
					if(temp != ""){
						commandParameters.push_back(temp);
					}
				}
			}
			return commandParameters;
		}
		
		struct CommandEntry{
			std::vector<std::string> pattern;
			std::function<bool(nlohmann::json&, std::vector<std::string>)> command;
		};
		
		bool showCommand(nlohmann::json& outputs, std::vector<std::string> command);
		
		bool configListCommand(nlohmann::json& outputs, std::vector<std::string> command);
		bool configGetCommand(nlohmann::json& outputs, std::vector<std::string> command);
		bool configSetCommand(nlohmann::json& outputs, std::vector<std::string> command);
		
		std::function<bool(nlohmann::json&, std::vector<std::string>)> makeTextCommand(std::string text){
			return [text](nlohmann::json& outputs, std::vector<std::string> command)->bool{
				printText(outputs, text);
				return true;
			};
		}
		
		const std::vector<CommandEntry> commandEntries = {
			{
				splitCommandString("help"),
				makeTextCommand(
					"Available Commands:\n"
					"  help                             show this help screen\n"
					"  about                            show information about this program\n"
					"  help <command>                   show help screen for a command\n"
					"  \n"
					"  show <article>                   view an article\n"
					"  \n"
					"  config                           list all configuration variables\n"
					"  config get <variable>            show the current value of a configuration variable\n"
					"  config set <variable> <value>    set a configuration variable to a specified value\n"
				)
			},
			{
				splitCommandString("about"),
				makeTextCommand(
					"  This program was created by DanielBatteryStapler and is based on the SCiPNET program in Dr. Clef's SCP-2317.\n"
					"  It can be used in-browser at https://scp.karagory.com/scipnet .\n"
					"  The program can also be used a stand-alone application, but doing so is more of an involved process.\n"
					"  Please visit the discord server for more information and support.\n"
					"  \n"
					"  Join the community! https://discord.gg/7GjqMJj\n"
					"  Bug reports, suggestions, and comments are welcome and encouraged.\n"
					"  \n"
					"  This program is a part of, and depends on, the SCP Conversion Project.\n"
					"  More information can be found on the project homepage, https://scp.karagory.com/\n"
					"  This program, as well as the entire SCP Conversion Project, is Free and Open-Source under the GPLv3 Copyright License.\n"
					"  \n"
					"  The copyright of the individual SCPs, tales, articles, and images are owned by their respective owners(not me).\n"
					"  \n"
					"  I would like to give thanks to:\n"
					"  * My Lovely Girlfriend, for supporting me\n"
					"  * Oliver Lugg, for making terrific music\n"
					"  * Benjamin Arnold, for getting me into C++ programming\n"
					"  * Every author of the SCP Wiki, for making a community worth converting\n"
				)
			},
			{
				splitCommandString("help about"),
				makeTextCommand(
					"Usage: about\n"
					"\n"
					"  shows information basic about this program, links to the project homepage and discord server, and copyright information\n"
				)
			},
			{
				splitCommandString("help help"),
				makeTextCommand(
					"Usage: help\n"
					"       help <command>\n"
					"\n"
					"  if a command is specified, it will print detailed information about that command\n"
					"  if no command is specified, it will print a list of available commands\n"
				)
			},
			{
				splitCommandString("help show"),
				makeTextCommand(
					"Usage: show <article>\n"
					"\n"
					"  shows the <article> in the command line article viewer\n"
					"  <article> must contain the entire article name, e.g. 'scp-1548' not just '1548'\n"
					"  up+down arrows scroll the viewer, q quits\n"
					"  left+right arrows allow the selecting of links and buttons\n"
					"  enter 'clicks' on the specified link or button\n"
					"  1+2 go backwards and forwards in page history\n"
				)
			},
			{
				splitCommandString("help config"),
				makeTextCommand(
					"Usage: config\n"
					"       config get <variable>\n"
					"       config set <variable> <value>\n"
					"\n"
					"  view and sets the values of configuration variables, changing variaious aspects of this program\n"
					"  first usage list all available variables and their values\n"
					"  second usage shows the value of a specific variable, a description, and what values are allowed\n"
					"  third usage sets a specific variable to the specified value\n"
					"  this command should be used with caution as variables can be set to values that may cause bugs, strange behaviors, and crashes\n"
				)
			},
			{
				splitCommandString("show *"),
				showCommand,
			},
			{
				splitCommandString("config"),
				configListCommand,
			},
			{
				splitCommandString("config get *"),
				configGetCommand,
			},
			{
				splitCommandString("config set * *"),
				configSetCommand,
			}
		};
			
		bool showCommand(nlohmann::json& outputs, std::vector<std::string> command){
			nlohmann::json output;
			output["type"] = "loadArticle";
			output["articleUrl"] = command[1];
			outputs.push_back(output);
			return false;
		}
		
		struct ConfigEntry{
			std::string name;
			std::string description;
			std::string allowedValues;
			std::function<std::string()> getValue;
			std::function<bool(std::string)> setValue;
		};
		
		std::string makeAllowedValues(bool* variable){
			return "true or false";
		}
		
		std::function<std::string()> makeGetValue(bool* variable){
			return [variable]()->std::string{
				if(*variable){
					return "true";
				}
				else{
					return "false";
				}
			};
		}
		
		std::function<bool(std::string)> makeSetValue(bool* variable){
			return [variable](std::string input)->bool{
				if(noCaseEquals(input, "true") || input == "1" || noCaseEquals(input, "yes") || noCaseEquals(input, "y")){
					*variable = true;
					return true;
				}
				else if(noCaseEquals(input, "false") || input == "0" || noCaseEquals(input, "no") || noCaseEquals(input, "n")){
					*variable = false;
					return true;
				}
				else{
					return false;
				}
			};
		}
		
		std::string makeAllowedValues(double* variables){
			return "any positive decimal number";
		}
		
		
		std::function<std::string()> makeGetValue(double* variable){
			return [variable]()->std::string{
				return std::to_string(*variable);
			};
		}
		
		std::function<bool(std::string)> makeSetValue(double* variable){
			return [variable](std::string input)->bool{
				try{
					double temp = std::stod(input);
					if(temp <= 0 || std::isfinite(temp) == false){
						return false;
					}
					*variable = temp;
					return true;
				}
				catch(std::exception& e){
					return false;
				}
			};
		}
		
		std::string makeAllowedValues(int* variables){
			return "any positive integer number";
		}
		
		
		std::function<std::string()> makeGetValue(int* variable){
			return [variable]()->std::string{
				return std::to_string(*variable);
			};
		}
		
		std::function<bool(std::string)> makeSetValue(int* variable){
			return [variable](std::string input)->bool{
				try{
					int temp = std::stoi(input);
					if(temp <= 0){
						return false;
					}
					*variable = temp;
					return true;
				}
				catch(std::exception& e){
					return false;
				}
			};
		}
		
		const std::vector<ConfigEntry> configVariables = {
			{
				"maxPageWidth",
				"Maximum width of a page in characters",
				makeAllowedValues(&Converter::maxPageWidth),
				makeGetValue(&Converter::maxPageWidth),
				makeSetValue(&Converter::maxPageWidth)
			},
			{
				"imageMaxWidthExpandCutoff",
				"Cutoff for when an image is large enough that it should be handled as if the image fills the entire page width, can be used to fix some page layout issues",
				makeAllowedValues(&Converter::imageMaxWidthExpandCutoff),
				makeGetValue(&Converter::imageMaxWidthExpandCutoff),
				makeSetValue(&Converter::imageMaxWidthExpandCutoff)
			},
			{
				"imageUseColors",
				"Should images use colors or be gray-scale only",
				makeAllowedValues(&Image::useColor),
				makeGetValue(&Image::useColor),
				makeSetValue(&Image::useColor)
			},
			{
				"imageUseFullColors",
				"Should images use full color spectrum or round colors together (enabling this make cause problems in some terminals)",
				makeAllowedValues(&Image::fullColor),
				makeGetValue(&Image::fullColor),
				makeSetValue(&Image::fullColor)
			},
			{
				"imagePixelGlyphWidth",
				"Width of a single glyph in an image compared to the pixels in the image",
				makeAllowedValues(&Image::pixelGlyphRatioHorizontal),
				makeGetValue(&Image::pixelGlyphRatioHorizontal),
				makeSetValue(&Image::pixelGlyphRatioHorizontal)
			},
			{
				"imagePixelGlyphHeight",
				"Height of a single glyph in an image compared to the pixels in the image",
				makeAllowedValues(&Image::pixelGlyphRatioVertical),
				makeGetValue(&Image::pixelGlyphRatioVertical),
				makeSetValue(&Image::pixelGlyphRatioVertical)
			}
		};
		
		bool configListCommand(nlohmann::json& outputs, std::vector<std::string> command){
			printText(outputs, "Configuration Variables:\n");
			for(const ConfigEntry& entry : configVariables){
				printText(outputs, "  " + entry.name + " = " + entry.getValue() + "\n");
			}
			return true;
		}
		
		bool configGetCommand(nlohmann::json& outputs, std::vector<std::string> command){
			for(const ConfigEntry& entry : configVariables){
				if(noCaseEquals(command[2], entry.name)){
					printText(outputs, "description: " + entry.description + "\n");
					printText(outputs, entry.name + " = " + entry.getValue() + "\n");
					printText(outputs, "allowed values: " + entry.allowedValues + "\n");
					return true;
				}
			}
			printText(outputs, "no such configuation variable\n");
			return true;
		}
		
		bool configSetCommand(nlohmann::json& outputs, std::vector<std::string> command){
			for(const ConfigEntry& entry : configVariables){
				if(noCaseEquals(command[2], entry.name)){
					if(entry.setValue(command[3])){
						printText(outputs, entry.name + " = " + entry.getValue() + "\n");
					}
					else{
						printText(outputs, "invalid value for configuration variable\n");
						printText(outputs, "allowed values: " + entry.allowedValues + "\n");
					}
					return true;
				}
			}
			printText(outputs, "no such configuation variable\n");
			return true;
		}
	}
	
	void handleCommand(nlohmann::json& outputs, std::string command){
		std::vector<std::string> commandParameteres = splitCommandString(command);
		
		bool printPrompt = true;
		
		if(commandParameteres.size() != 0){
			bool matchesAny = false;
			for(const auto& entry : commandEntries){
				bool matches = true;
				if(entry.pattern.size() == commandParameteres.size()){
					for(int i = 0; i < entry.pattern.size(); i++){
						if(entry.pattern[i] != "*" && entry.pattern[i] != commandParameteres[i]){
							matches = false;
							break;
						}
					}
				}
				else{
					matches = false;
				}
				
				if(matches){
					matchesAny = true;
					printPrompt = entry.command(outputs, commandParameteres);
					break;
				}
			}
			
			if(matchesAny == false){
				printPrompt = true;
				printText(outputs, "unknown command\n");
			}
		}
		if(printPrompt){
			printText(outputs, "SCiPNET> ");
		}
	}
}
