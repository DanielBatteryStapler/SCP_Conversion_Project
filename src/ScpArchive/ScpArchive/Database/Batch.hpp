#ifndef BATCH_HPP
#define BATCH_HPP

#include "Importer.hpp"

namespace Importer{
	void handleBatches(std::string batchesFolder, std::string batchDataFile, bool automatic = false);
	void importBatch(std::string batchFolder);
}

#endif // BATCH_HPP
