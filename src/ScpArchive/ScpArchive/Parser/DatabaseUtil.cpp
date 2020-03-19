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
			out.name = author.name;
		}
		else{
			out.type = ShownAuthor::Type::Deleted;
		}
		return out;
	}
}
