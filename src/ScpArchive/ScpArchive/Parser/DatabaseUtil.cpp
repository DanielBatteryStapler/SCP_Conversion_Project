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
	
    PageInfo getPageInfo(Database* db, Database::ID pageId){
		PageInfo info;
		{
			Database::PageRevision latest = db->getLatestPageRevision(pageId);
			info.title = latest.title;
		}
		{
			Database::PageRevision oldest = db->getPageRevision(db->getPageRevisions(pageId).back());
			if(oldest.authorId){
				Database::Author author = db->getAuthor(*oldest.authorId);
				if(author.type == Database::Author::Type::User && author.name != ""){
					info.author = author.name;
				}
			}
			info.creationTimeStamp = oldest.timeStamp;
		}
		info.name = db->getPageName(pageId);
		info.parent = db->getPageParent(pageId);
        info.tags = db->getPageTags(pageId);
        info.rating = db->getPageRating(pageId);
        info.votes = db->getPageVotesCount(pageId);
		return info;
    }
}
