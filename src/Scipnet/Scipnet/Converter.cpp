#include "Converter.hpp"

#include <sstream>
#include <algorithm>
#include "ColorTable.hpp"

#include "Helpers.hpp"

namespace Converter{
	
	namespace{
		int parseSingleHex(char c){
			switch(tolower(c)){
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;
			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;
			case '8': return 8;
			case '9': return 9;
			case 'a': return 10;
			case 'b': return 11;
			case 'c': return 12;
			case 'd': return 13;
			case 'e': return 14;
			case 'f': return 15;
			default: throw std::runtime_error{"invalid hex char"};
			}
		}
		
		int parseHex(std::string hex){
			if(hex.size() == 1){
				return parseSingleHex(hex[0]);
			}
			else if(hex.size() == 2){
				return parseSingleHex(hex[0]) * 16 + parseSingleHex(hex[1]);
			}
			else{
				throw std::runtime_error{"invalid hex string"};
			}
		}
		
		// trim from start (in place)
		static inline void ltrim(std::string &s) {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
				return !std::isspace(ch);
			}));
		}

		// trim from end (in place)
		static inline void rtrim(std::string &s) {
			s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
				return !std::isspace(ch);
			}).base(), s.end());
		}

		// trim from both ends (in place)
		static inline void trim(std::string &s) {
			ltrim(s);
			rtrim(s);
		}
		
		void setColor(GlyphAttributes& attr, std::string color){
			trim(color);
			
			if(color != ""){
				std::transform(color.begin(), color.end(), color.begin(), ::tolower);
				
				if(colorTable.find(color) != colorTable.end()){
					color = colorTable[color];
				}
				
				std::transform(color.begin(), color.end(), color.begin(), ::tolower);
				
				if(color[0] == '#'){
					color = color.substr(1);
				}
				
				bool isHexColor = true;
				for(char c : color){
                    switch(c){
                        case '0':case '1':case '2':case '3':case '4':
                        case '5':case '6':case '7':case '8':case '9':
                        case 'a':case 'b':case 'c':case 'd':case 'e':
                        case 'f':
                            break;
                        default:
                            isHexColor = false;
                            break;
                    }
                }
				
				if(isHexColor){
					if(color.size() == 6){
						attr.color.red = parseHex(color.substr(0, 2));
						attr.color.green = parseHex(color.substr(2, 2));
						attr.color.blue = parseHex(color.substr(4, 2));
						attr.colorSet = true;
					}
					else if(color.size() == 3){
						attr.color.red = parseHex(color.substr(0, 1)) * 16;
						attr.color.green = parseHex(color.substr(1, 1)) * 16;
						attr.color.blue = parseHex(color.substr(2, 1)) * 16;
						attr.colorSet = true;
					}
				}
			}
		}
		
		void add(std::vector<std::shared_ptr<Element>>& stack, std::shared_ptr<Element> e){
			if(stack.size() == 0){
				throw std::runtime_error("attempting to add to empty stack");
			}
			stack.back()->addElement(e);
		}
		
		void push(std::vector<std::shared_ptr<Element>>& stack, std::shared_ptr<Element> e){
			stack.push_back(e);
		}
		
		void pop(std::vector<std::shared_ptr<Element>>& stack){
			std::shared_ptr<Element> e = stack.back();
			stack.pop_back();
			add(stack, e);
		}
		
		struct GlobalContext{
			
		};
		
		struct LocalContext{
			GlyphAttributes attributes = {};
			std::string link;
		};
		
		std::shared_ptr<Element> popWithoutAdd(std::vector<std::shared_ptr<Element>>& stack){
			std::shared_ptr<Element> e = stack.back();
			stack.pop_back();
			return e;
		}
		
		void addText(std::vector<std::shared_ptr<Element>>& stack, std::string text, LocalContext localContext){
			if(text.size() > 0 && text[0] == ' '){
				text = text.substr(1);
				add(stack, makeText(" ", localContext.attributes, localContext.link));
			}
			std::string temp;
			for(char c : text){
				if(c == ' '){
					if(temp != ""){
						add(stack, makeText(temp, localContext.attributes, localContext.link));
						add(stack, makeText(" ", localContext.attributes, localContext.link));
						temp = "";
					}
				}
				else{
					temp += c;
				}
			}
			if(temp != ""){
				add(stack, makeText(temp, localContext.attributes, localContext.link));
				temp = "";
			}
		}
		
		void convertElement(std::vector<std::shared_ptr<Element>>& stack, const nlohmann::json& page, GlobalContext& globalContext, LocalContext localContext);
		
		void convertBranches(std::vector<std::shared_ptr<Element>>& stack, const nlohmann::json& page, GlobalContext& globalContext, LocalContext localContext){
			for(const auto& branch : page["branches"]){
				convertElement(stack, branch, globalContext, localContext);
			}
		}
		
		void convertElement(std::vector<std::shared_ptr<Element>>& stack, const nlohmann::json& page, GlobalContext& globalContext, LocalContext localContext){
			std::string type = page["type"].get<std::string>();
			
			if(type == "PlainText"){
				addText(stack, page["data"].get<std::string>(), localContext);
			}
			if(type == "LiteralText"){
				addText(stack, page["data"].get<std::string>(), localContext);
			}
			else if(type == "HyperLink"){
				localContext.attributes.underline = true;
				localContext.link = redirectLink(page["data"]["url"].get<std::string>());
				add(stack, makeText(page["data"]["shownText"].get<std::string>(), localContext.attributes, localContext.link));
				//addText(stack, page["data"]["shownText"].get<std::string>(), localContext);
			}
			else if(type == "User"){
				addText(stack, page["data"]["shownName"].get<std::string>(), localContext);
			}
			else if(type == "LineBreak"){
				add(stack, makeText(""));
				add(stack, makeNewline());
			}
			else if(type == "Span"){
				std::string color;
				if(page["data"].find("style") != page["data"].end()){
					std::string styleTag = page["data"]["style"].get<std::string>();
					if(styleTag.find("color:") != std::string::npos){
						if(styleTag.find(";", styleTag.find("color:")) == std::string::npos){
							color = styleTag.substr(styleTag.find("color:") + 6);
						}
						else{
							color = styleTag.substr(styleTag.find("color:") + 6, styleTag.find(";", styleTag.find("color:")) - styleTag.find("color:") - 6);
						}
					}
				}
				
				setColor(localContext.attributes, color);
				convertBranches(stack, page, globalContext, localContext);
			}
			else if(type == "Paragraph"){
				push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 1, 0));
				convertBranches(stack, page, globalContext, localContext);
				pop(stack);
			}
			else if(type == "QuoteBox"){
				push(stack, makeDivBlock(Element::Align::Left, {}, 0, 1, 1, 1));
				push(stack, makeQuoteBox());
				push(stack, makeDivBlock(Element::Align::Left, {}, 1, 1, 0, 1));
				convertBranches(stack, page, globalContext, localContext);
				pop(stack);
				pop(stack);
				pop(stack);
			}
			else if(type == "Divider"){
				std::string dividerType = page["data"].get<std::string>();
				if(dividerType == "Line"){
					//add(stack, makeDivBlock(Element::Align::Left, {makeInlineBlock({makeHorizontalLine()}, 0, 0, 0, 0)}, 0, 0, 1, 0));
					add(stack, makeDivBlock(Element::Align::Left, {makeInlineBlock({}, 0, 8, 0, 0), makeHorizontalLine()}, 0, 0, 1, 0));
				}
				else if(dividerType == "ClearBoth" || dividerType == "ClearLeft" || dividerType == "ClearRight"){
					add(stack, makeClearLine());
				}
			}
			else if(type == "StyleFormat"){
				
				std::string styleType = page["data"]["type"].get<std::string>();
				
				if(styleType == "Bold"){
					localContext.attributes.bold = true;
				}
				else if(styleType == "Underline"){
					localContext.attributes.underline = true;
				}
				else if(styleType == "Italics"){
					localContext.attributes.italic = true;
				}
				else if(styleType == "Color"){
					setColor(localContext.attributes, page["data"]["color"].get<std::string>());
				}
				else if(styleType == "Strike"){
					addText(stack, "~~", localContext);
				}
				
				convertBranches(stack, page, globalContext, localContext);
				
				if(styleType == "Strike"){
					addText(stack, "~~", localContext);
				}
			}
			else if(type == "List"){
				int numbered = -1;
				if(page["data"].get<std::string>() == "Number"){
					numbered = 1;
				}
				
				std::shared_ptr<Table> table = makeTable(false, false);
				
				for(const auto& listElement : page["branches"]){
					
					Table::Cell cellLeft;
					cellLeft.columnSpan = 1;
					if(numbered == -1){
						cellLeft.element = makeText(" * ");
					}
					else{
						cellLeft.element = makeText(" " + std::to_string(numbered) + ". ");
						numbered++;
					}
					
					Table::Cell cellRight;
					cellRight.columnSpan = 1;
					
					push(stack, makeInlineBlock({}, 0, 0, 0, 0));
					convertBranches(stack, listElement, globalContext, localContext);
					cellRight.element = popWithoutAdd(stack);
					
					table->tableRows.push_back({cellLeft, cellRight});
				}
				
				add(stack, makeDivBlock(Element::Align::Left, {table}, 0, 0, 1, 0));
			}
			else if(type == "Table"){
				std::shared_ptr<Table> table = makeTable(true, true);
				
				for(const auto& rowJson : page["branches"]){
					std::vector<Table::Cell> tableRow;
					
					for(const auto& cellJson : rowJson["branches"]){
						push(stack, makeInlineBlock({}, 0, 0, 0, 0));
						
						convertBranches(stack, cellJson, globalContext, localContext);
						
						Table::Cell cell;
						cell.columnSpan = cellJson["data"]["span"].get<int>();
						cell.element = popWithoutAdd(stack);
						tableRow.push_back(cell);
					}
					
					table->tableRows.push_back(tableRow);
				}
				
				add(stack, makeClearLine());
				add(stack, makeDivBlock(Element::Align::Left, {table}, 1, 1, 1, 1));
			}
			else if(type == "Collapsible"){
				push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0));
				convertBranches(stack, page, globalContext, localContext);
				
				localContext.attributes.underline = true;
				std::shared_ptr<Element> closedText = makeText(page["data"]["closedText"].get<std::string>(), localContext.attributes);
				std::shared_ptr<Element> openedText = makeText(page["data"]["openedText"].get<std::string>(), localContext.attributes);
				bool isOpen = page["data"]["defaultShow"].get<bool>();
				add(stack, makeCollapsible(closedText, openedText, popWithoutAdd(stack), isOpen));
			}
			else if(type == "TabView"){
				
				std::vector<TabView::Tab> tabs;
				
				for(const auto& tabJson : page["branches"]){
					push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0));
					convertBranches(stack, tabJson, globalContext, localContext);
					TabView::Tab tab;
					tab.content = popWithoutAdd(stack);
					tab.text = makeText(tabJson["data"].get<std::string>(), localContext.attributes, "");
					tabs.push_back(tab);
				}
				
				add(stack, makeClearLine());
				add(stack, makeTabView(tabs));
			}
			else if(type == "Heading"){
				std::shared_ptr<Table> table = makeTable(false, false);
				
				Table::Cell cellLeft;
				cellLeft.columnSpan = 1;
				std::string text;
				for(int i = 0; i < page["data"]["degree"].get<int>(); i++){
					text += "#";
				}
				text += " ";
				cellLeft.element = makeText(text);
				
				Table::Cell cellRight;
				cellRight.columnSpan = 1;
				
				push(stack, makeInlineBlock({}, 0, 0, 0, 0));
				convertBranches(stack, page, globalContext, localContext);
				cellRight.element = popWithoutAdd(stack);
				
				table->tableRows.push_back({cellLeft, cellRight});
				
				add(stack, makeDivBlock(Element::Align::Left, {table}, 0, 0, 1, 1));
			}
			else if(type == "FootNote"){
				add(stack, makeText("[" + std::to_string(page["data"].get<int>()) + "]"));
			}
			else if(type == "FootNoteBlock"){
				if(page["branches"].size() > 0){
					push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 1, 1));
					addText(stack, "# " + page["data"].get<std::string>(), localContext);
					pop(stack);
					
					std::shared_ptr<Table> table = makeTable(false, false);
					for(const auto& footNote : page["branches"]){
						Table::Cell cellLeft;
						cellLeft.columnSpan = 1;
						cellLeft.element = makeText(" [" + std::to_string(footNote["data"].get<int>()) + "]. ");
						
						Table::Cell cellRight;
						cellRight.columnSpan = 1;
						
						push(stack, makeInlineBlock({}, 0, 0, 0, 0));
						convertBranches(stack, footNote, globalContext, localContext);
						cellRight.element = popWithoutAdd(stack);
						
						table->tableRows.push_back({cellLeft, cellRight});
					}
					add(stack, makeDivBlock(Element::Align::Left, {table}, 0, 0, 1, 0));
				}
			}
			else if(type == "Div"){
				if(page["data"].find("class") != page["data"].end()){
					if(page["data"]["class"].get<std::string>().find("scp-image-block") != std::string::npos){
						
						std::shared_ptr<Table> table = makeTable(true, true);
						
						
						push(stack, makeInlineBlock({}, 0, 0, 0, 0));
						convertBranches(stack, page["branches"][0], globalContext, localContext);
						std::shared_ptr<Element> image = popWithoutAdd(stack);
						
						push(stack, makeInlineBlock({}, 0, 0, 0, 0));
						convertBranches(stack, page["branches"][1]["branches"][0], globalContext, localContext);
						std::shared_ptr<Element> caption = popWithoutAdd(stack);
						
						add(stack, makeFloat(Element::Direction::Right, {
							makeInlineBlock({makeImageCaptionTable(image, caption)}, 0, 0, 1, 1)
						}));
					}
					else if(page["data"]["class"].get<std::string>().find("anom-bar-container") != std::string::npos){
						//those nice memo headers don't render correctly, like at all, and the only way for them to would be to make
						//a custom layout element specifically for them, but I haven't done that yet
						//for now, we're just going to have to delete them from the page so it at least doesn't screw up the page
					}
					else{
						push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0));
						convertBranches(stack, page, globalContext, localContext);
						pop(stack);
					}
				}
				else{
					push(stack, makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0));
					convertBranches(stack, page, globalContext, localContext);
					pop(stack);
				}
			}
			else if(type == "Image"){
				if(page["imageData"]["valid"].get<bool>() == true){
					int imageWidth = page["imageData"]["width"].get<int>();
					int imageHeight = page["imageData"]["height"].get<int>();
					std::string pixelString = page["imageData"]["pixels"].get<std::string>();
					
					std::vector<Image::Pixel> pixelData;
					for(int i = 0; i < pixelString.size(); i += 6){
						Image::Pixel pixel;
						pixel.red = parseHex(pixelString.substr(i, 2));
						pixel.green = parseHex(pixelString.substr(i + 2, 2));
						pixel.blue = parseHex(pixelString.substr(i + 4, 2));
						pixelData.push_back(pixel);
					}
					
					int maxWidth = imageWidth / Image::pixelGlyphRatioHorizontal;
					
					std::string style = page["data"]["style"].get<std::string>();
					
					std::size_t widthPos = style.find("width");
					std::size_t colonPos = std::string::npos;
					if(widthPos != std::string::npos){
						colonPos = style.find(":", widthPos);
					}
					
					if(maxWidth >= imageMaxWidthExpandCutoff){
						maxWidth = maxPageWidth;
					}
										
					if(colonPos != std::string::npos){
						std::size_t semicolonPos = style.find(";", colonPos);
						std::string widthString;
						if(semicolonPos == std::string::npos){
							widthString = style.substr(colonPos + 1);
						}
						else{
							widthString = style.substr(colonPos + 1, semicolonPos - colonPos - 1);
						}
						if(widthString.find("px") != std::string::npos){
							try{
								double widthPixels = std::stod(widthString);
								maxWidth = widthPixels / Image::pixelGlyphRatioHorizontal;
								if(maxWidth >= imageMaxWidthExpandCutoff){
									maxWidth = maxPageWidth;
								}
							}
							catch(std::exception& e){
								//do nothing
							}
						}
						else if(widthString.find("%") != std::string::npos){
							try{
								double widthPercent = std::stod(widthString);
								maxWidth = widthPercent / 100 * maxPageWidth;
							}
							catch(std::exception& e){
								//do nothing
							}
						}
					}
					
					add(stack, makeImage(imageWidth, imageHeight, pixelData, maxWidth));
				}
				else{
					int width = 25;
					int height = 25;
					std::vector<Image::Pixel> pixelData;
					for(int y = 0; y < width; y++){
						for(int x = 0; x < width; x++){
							Image::Pixel pixel;
							if(std::abs(x - y) <= 2 || std::abs(width - x - 1 - y) <= 2){
								pixel.red = 255;
								pixel.green = 200;
								pixel.blue = 200;
							}
							else{
								pixel.red = 0;
								pixel.green = 0;
								pixel.blue = 0;
							}
							pixelData.push_back(pixel);
						}
					}
					
					add(stack, makeImage(width, height, pixelData, width));
				}
			}
			else{
				convertBranches(stack, page, globalContext, localContext);
			}
		}
	}
	
	std::shared_ptr<Element> makeLayoutFromPageJson(const nlohmann::json& page){
		std::vector<std::shared_ptr<Element>> stack;
		stack.push_back(makeLimitedBlock(Element::Align::Left, {}, maxPageWidth, true));
		//stack.push_back(makeDivBlock(Element::Align::Left, {}, 0, 0, 0, 0));
		
		GlobalContext globalContext;
		LocalContext localContext;
		
		convertElement(stack, page["pageTree"], globalContext, localContext);
		
		return stack[0];
	}
}
