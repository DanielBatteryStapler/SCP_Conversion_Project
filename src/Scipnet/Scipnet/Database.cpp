#include "Database.hpp"

#include <sstream>
#include <iomanip>

#include <Magick++.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <curl/curl.h>
#include "Helpers.hpp"

namespace Database{
	nlohmann::json getPageTree(std::string name){
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		request.setOpt<curlpp::options::Url>("https://scp.karagory.com/" + name + "/showTreeJSON");
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		return nlohmann::json::parse(data);
	}

	std::string downloadImage(std::string url){
		curlpp::Cleanup myCleanup;
		curlpp::Easy request;
		if(url.find("/") == 0){
			request.setOpt<curlpp::options::Url>("https://scp.karagory.com/" + url);
		}
		else{
			request.setOpt<curlpp::options::Url>(url);
		}
		std::ostringstream os;
		curlpp::options::WriteStream ws(&os);
		request.setOpt(ws);
		request.perform();
		std::string data = os.str();
		return data;
	}

	nlohmann::json getPage(std::string name){
		nlohmann::json output;
		
		try{
			nlohmann::json pageTree = getPageTree(name);
			loadImagesIntoPageTree(pageTree);
			output["pageTree"] = pageTree;
		}
		catch(std::exception& e){
			output["pageTree"] = {
				{"type", "RootPage"},
				{"data", nullptr},
				{"branches", {
					{
						{"type", "Paragraph"},
						{"data", nullptr},
						{"branches", {
							{
								{"type", "PlainText"},
								{"data", "Error while loading page '" + name + "'"},
								{"branches", nlohmann::json::array()}
							}
						}}
					}
				}}
			};
		}
		
		return output;
	}

	namespace{

		void visitPageTree(nlohmann::json& pageTree, std::function<void(nlohmann::json&)> visitor){
			visitor(pageTree);
			for(auto& branch : pageTree["branches"]){
				visitPageTree(branch, visitor);
			}
		}

	}

	void loadImagesIntoPageTree(nlohmann::json& pageTree){

		visitPageTree(pageTree, [&](nlohmann::json& node){
			if(node["type"].get<std::string>() == "Image"){
				
				std::string rawImageFile = downloadImage(redirectLink(node["data"]["source"].get<std::string>()));
				
				try{
					Magick::Blob blob{rawImageFile.c_str(), rawImageFile.size()};
					Magick::Image image{blob};
					
					//image.quantizeColorSpace(Magick::RGBColorspace);
					//image.quantizeColors(256);
					//image.quantize();
					
					unsigned int rows = image.rows();
					unsigned int columns = image.columns();
					
					constexpr unsigned int maximumDimension = 256;
					
					if(std::max(rows, columns) > maximumDimension){
						if(rows > columns){
							columns = columns * static_cast<float>(maximumDimension)/rows;
							rows = maximumDimension;
						}
						else{
							rows = rows * static_cast<float>(maximumDimension)/columns;
							columns = maximumDimension;
						}
					}
					
					image.resize(Magick::Geometry{columns, rows});
					columns = image.columns();
					rows = image.rows();
					
					std::stringstream hexData;
					
					constexpr auto toHex = [](int num)->std::string{
						std::stringstream out;
						out << std::hex << (num / 16) << (num % 16);
						return out.str();
					};
					
					const Magick::PixelPacket* pixel_cache = image.getPixels(0, 0, columns, rows);
					
					for(unsigned int y = 0; y < rows; y++){
						for(unsigned int x = 0; x < columns; x++){
							const Magick::PixelPacket* pixel = (pixel_cache+y*columns+x);
							unsigned int depthCorrection = MaxRGB / 255;
							std::stringstream singlePixel;
							singlePixel << toHex(pixel->red / depthCorrection) << toHex(pixel->green / depthCorrection) << toHex(pixel->blue / depthCorrection);
							if(singlePixel.str().size() != 6){
								throw std::runtime_error{"invalid pixel: " + singlePixel.str()};
							}
							hexData << singlePixel.str();
						}
					}
					
					nlohmann::json imageJson;
					imageJson["valid"] = true;
					imageJson["pixels"] = hexData.str();
					imageJson["height"] = rows;
					imageJson["width"] = columns;
					node["imageData"] = imageJson;
				}
				catch(std::exception& e){
				
					nlohmann::json imageJson;
					imageJson["valid"] = false;
					node["imageData"] = imageJson;
				}
			}
		});

	}
}

















