#ifndef DATABASEUTIL_HPP
#define DATABASEUTIL_HPP

#include "Treer.hpp"
#include "../Database/Database.hpp"

namespace Parser{
    ShownAuthor getShownAuthor(Database* db, std::optional<Database::ID> authorId);
    PageInfo getPageInfo(Database* db, Database::ID pageId);
}

#endif // DATABASEUTIL_HPP
