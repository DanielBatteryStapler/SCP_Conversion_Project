#ifndef PDFCONVERTER_HPP
#define PDFCONVERTER_HPP

#include <string>
#include "../Database/Database.hpp"

namespace PDF{
	void doPDFConversionForPage(Database* db, std::string pageName, std::string tempFolder, std::string pdfCollectionFolder);
	void createZIPArchive(std::string pdfCollectionFolder, std::string tempFolder, std::string zipArchiveFile);
	void updatePDFs();
}

#endif // PDFCONVERTER_HPP
