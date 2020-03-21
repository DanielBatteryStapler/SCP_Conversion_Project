#include "DatabaseUtil.hpp"

namespace Parser{
	ShownAuthor getShownAuthor(Database* db, std::optional<Database::ID> authorId){
		ShownAuthor out;
		if(authorId){
			Database::Author author = db->getAuthor(authorId.value());
			switch(author.type){
				default:
					throw std::runtime_error("Got Author from Database with invalid Author::Type");
					break;
				case Database::Author::Type::System:
					out.type = ShownAuthor::Type::System;
					break;
				case Database::Author::Type::User:
					out.type = ShownAuthor::Type::User;
					break;
			}
			if(author.name == ""){//if the author has a blank name, that means it should be treated as deleted
				out.type = ShownAuthor::Type::Deleted;
			}
			else{
				out.shownName = author.name;
				out.linkName = normalizePageName(out.shownName);
			}
		}
		else{
			out.type = ShownAuthor::Type::Deleted;
		}
		return out;
	}
}
