#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include "Database.hpp"

namespace Importer{
	
	void importAllDataFromArchiveDirectory(Database database, std::string archiveDirectory);
	
}

#endif // IMPORTER_HPP
