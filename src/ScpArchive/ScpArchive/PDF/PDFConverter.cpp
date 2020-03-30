#include "PDFConverter.hpp"

#include "../Parser/Treer.hpp"
#include "../Parser/HTMLConverter.hpp"
#include "../Database/Database.hpp"
#include "../Parser/DatabaseUtil.hpp"
#include "../Database/Json.hpp"

#include <fstream>
#include <boost/filesystem.hpp>

namespace PDF{
	void doPDFConversionForPage(Database* db, std::string pageName, std::string tempFolder, std::string pdfCollectionFolder){
		std::string tempPageFolder = tempFolder + pageName + "/";
		std::string pageFilesFolder = tempPageFolder + "files/";
		std::string tempHTMLFile = tempPageFolder + "tempHtml.html";
		std::string tempPDFFile = tempPageFolder + "tempPdf.pdf";
		std::string finishedPdfFile = pdfCollectionFolder + pageName + ".pdf";
		
		boost::filesystem::remove_all(tempHTMLFile);
		boost::filesystem::create_directory(tempPageFolder);
		
		{
			Database::ID pageId = db->getPageId(pageName).value();
			Database::PageRevision revision = db->getLatestPageRevision(pageId);
			
			Parser::ParserParameters parserParameters = {};
			parserParameters.database = db;
			parserParameters.page = Parser::getPageInfo(db, pageId);
			
			Parser::PageTree pageTree = Parser::makeTreeFromPage(revision.sourceCode, parserParameters);
				
			std::ofstream tempHTML{tempHTMLFile};
			MarkupOutStream htmlOut{&tempHTML};
			htmlOut << "<!DOCTYPE html><html><head>"_AM
			<< "<meta http-equiv='Content-Security-Policy' content='upgrade-insecure-requests'>"_AM
			<< "<link rel='stylesheet' type='text/css' href='"_AM << Config::getWebRootStaticFolder() << "sigma9Backup.css'>"_AM
			<< "<link rel='stylesheet' type='text/css' href='"_AM << Config::getWebRootStaticFolder() << "style.css'>"_AM;
			for(const Parser::CSS& css : pageTree.cssData){
				htmlOut << "<style>"_AM << allowMarkup(css.data) << "</style>"_AM;///TODO: fix allow markup security hole
			}
			htmlOut << "<link rel='stylesheet' type='text/css' href='"_AM << Config::getWebRootStaticFolder() << "pdfStyle.css'>"_AM
			<< "<meta charset='UTF-8'></head><body><div id='article'><h1>"_AM << revision.title << "</h1><hr>"_AM;
			Parser::convertPageTreeToHtml(htmlOut, pageTree, true);
			htmlOut << "</div></body></html>"_AM;
			tempHTML.close();
			
			boost::filesystem::create_directory(pageFilesFolder);
			std::vector<Database::ID> pageFiles = db->getPageFiles(pageId);
			for(Database::ID pageFileId : pageFiles){
				Database::PageFile pageFile = db->getPageFile(pageFileId);
				std::ofstream pageFileOutput{pageFilesFolder + pageFile.name};
				db->downloadPageFile(pageFileId, pageFileOutput);
				pageFileOutput.close();
			}
		}
		
		///this is calling std::system, but this code is never called by the Website, it can only be called if you tell the program to call it
		//everything will probably be fine, probably
		
		std::system(("wkhtmltopdf -T 10mm -R 10mm -B 10mm -L 10mm --zoom 1.10 \"" + tempHTMLFile + "\" \"" + tempPDFFile + "\"").c_str());
		
		boost::filesystem::rename(tempPDFFile, finishedPdfFile);
		boost::filesystem::remove_all(tempPageFolder);
	}
	
	void createZIPArchive(std::string pdfCollectionFolder, std::string tempFolder, std::string zipArchiveFile){
		
		std::string tempZipFile = tempFolder + "fullArchive.zip";
		boost::filesystem::remove_all(tempZipFile);
		
		std::cout << "Making ZIP Archive...\n";
		std::system(("7z a \"" + tempZipFile + "\" \"" + pdfCollectionFolder + "*.zip\"").c_str());
		boost::filesystem::remove_all(zipArchiveFile);
		boost::filesystem::rename(tempZipFile, zipArchiveFile);
		std::cout << "ZIP Archive complete.\n";
	}
	
	namespace{
		bool pageNeedsUpdate(const nlohmann::json& pdfData, std::string pageName, std::int64_t newRevisionIndex){
			for(const nlohmann::json& entry : pdfData["pages"]){
				if(entry["pageName"].get<std::string>() == pageName){
					if(entry["revisionIndex"].get<std::int64_t>() == newRevisionIndex){
						return false;
					}
					else{
						return true;
					}
				}
			}
			return true;
		}
		
		void setPageUpdated(nlohmann::json& pdfData, std::string pageName, std::int64_t newRevisionIndex){
			for(nlohmann::json& entry : pdfData["pages"]){
				if(entry["pageName"].get<std::string>() == pageName){
					entry["revisionIndex"] = newRevisionIndex;
					return;
				}
			}
			{
				nlohmann::json newEntry;
				newEntry["pageName"] = pageName;
				newEntry["revisionIndex"] = newRevisionIndex;
				pdfData["pages"].push_back(newEntry);
			}
		}
		
		struct UpdateEntry{
			std::string pageName;
			std::int64_t revisionIndex;
		};
	}
	
	void updatePDFs(){
		std::unique_ptr<Database> db = Database::connectToDatabase(Config::getProductionDatabaseName());
		
		std::string pdfFolder = Config::getPDFFolder();
		std::string pdfCollectionFolder = pdfFolder + "collection/";
		std::string tempFolder = pdfFolder + "temp/";
		std::string pdfDataFile = pdfFolder + "pdfData.json";
		std::string zipArchiveFile = pdfFolder + "full.zip";
		
		nlohmann::json pdfData = Json::loadJsonFromFile(pdfDataFile);
		
		std::vector<UpdateEntry> updatesNeeded;
		{
			std::vector<Database::ID> pageList = db->getPageList();
			for(Database::ID pageId : pageList){
				std::string pageName = db->getPageName(pageId);
				std::int64_t newestRevisionIndex = db->getPageRevisions(pageId).size() - 1;
				if(pageNeedsUpdate(pdfData, pageName, newestRevisionIndex)){
					updatesNeeded.push_back({pageName, newestRevisionIndex});
				}
			}
		}
		
		if(updatesNeeded.size() == 0){
			std::cout << "No PDF updates needed.\n";
			return;
		}
		
		std::cout << "Updates needed on pages\n";
		for(const UpdateEntry& entry : updatesNeeded){
			std::cout << "    " << entry.pageName << " -> " << entry.revisionIndex << "\n";
		}
		
		std::cout << "\nDoing updates...\n";
		
		std::int64_t updateCount = 1;
		for(const UpdateEntry& entry : updatesNeeded){
			std::cout << "    (" << updateCount << "/" << updatesNeeded.size() << ") Converting " << entry.pageName << " -> " << entry.revisionIndex << "...\n";
			doPDFConversionForPage(db.get(), entry.pageName, tempFolder, pdfCollectionFolder);
			updateCount++;
		}
		
		std::cout << "\nCreating ZIP archive...\n";
		createZIPArchive(pdfCollectionFolder, tempFolder, zipArchiveFile);
		
		std::cout << "\nMarking and saving updates...\n";
		for(const UpdateEntry& entry : updatesNeeded){
			setPageUpdated(pdfData, entry.pageName, entry.revisionIndex);
		}
		Json::saveJsonToFile(pdfDataFile, pdfData);
		std::cout << "Update complete.\n";
	}
}
