#include "ListPagesRuleSet.hpp"

#include "../../DatabaseUtil.hpp"
#include "../../../Database/Database.hpp"

#include <ctime>
#include "../../Templater.hpp"

namespace{
	inline std::vector<std::string> splitString(const std::string& raw){
		std::vector<std::string> out;
		std::string buffer;
		const auto emptyBuffer = [&buffer, &out](){
			if(buffer.size() > 0){
				out.push_back(buffer);
				buffer.clear();
			}
		};
		for(char c : raw){
			switch(c){
				case ',':
				case ' ':
					emptyBuffer();
					break;
				default:
					buffer += c;
					break;
			}
		}
		emptyBuffer();
		/*
		std::cout << "Split string \"" << raw << "\" into {";
		for(std::string str : out){std::cout << "\"" << str << "\" ";}
		std::cout << "}\n";
		*/
		return out;
	}
	
	inline std::string getPageCategory(const std::string& fullName){
		std::size_t pos = fullName.find(":");
		if(pos == std::string::npos){
			return "";
		}
		else{
			return fullName.substr(0, pos);
		}
	}
	
	inline std::string getPageName(const std::string& fullName){
		std::size_t pos = fullName.find(":");
		if(pos == std::string::npos){
			return fullName;
		}
		else{
			return fullName.substr(pos + 1, fullName.size() - pos - 1);
		}
	}
	
	inline void chopFront(std::string& text, std::size_t pos){
		text = text.substr(pos, text.size() - pos);
		Parser::trimString(text);
	}
	
	enum class DateCode{Error, Year, Month};
	
	inline DateCode parseDate(const std::string& text, TimeStamp& out){
		if(text.size() == 4){
			int year;
			try{
				year = std::stoi(text);
				
				std::tm time = {};
				time.tm_year = year - 1900;
				
				out = static_cast<std::int64_t>(timegm(&time));
				
				return DateCode::Year;
			}
			catch(std::exception& e){
				//ignore it
			}
		}
		else if(text.size() == 7 && text.find('.') != std::string::npos){
			int year;
			int month;
			try{
				year = std::stoi(text.substr(0, text.find('.')));
				std::cout << year << "\n";
				std::string temp = text;
				chopFront(temp, temp.find('.') + 1);
				month = std::stoi(temp);
				
				std::tm time = {};
				time.tm_year = year - 1900;
				time.tm_mon = month - 1;
				
				out = static_cast<std::int64_t>(timegm(&time));
				
				return DateCode::Month;
			}
			catch(std::exception& e){
				//ignore it
			}
		}
		return DateCode::Error;
	}
	
	inline bool parseDateRangeNumber(const std::string& text, TimeStamp& out){
		out = getCurrentTime();
		std::int64_t number = 1;
		{
			std::string numberStr;
			for(char c : text){
				if(isdigit(c)){
					numberStr += c;
				}
			}
			if(numberStr != ""){
				try{
					number = std::stoll(numberStr);
				}
				catch(std::exception& e){
					//ignore error
				}
			}
		}
		if(text.find("hour") != std::string::npos){
			out -= number * 60 * 60;
		}
		else if(text.find("day") != std::string::npos){
			out -= number * 60 * 60 * 24;
		}
		else if(text.find("week") != std::string::npos){
			out -= number * 60 * 60 * 24 * 7;
		}
		else if(text.find("month") != std::string::npos){
			out -= number * 60 * 60 * 24 * 30;
		}
		else{
			return false;
		}
		return true;
	}
	
	void handleUrlParameter(std::string& parameter, std::string name, const Parser::TreeContext& context){
		if(Parser::check(parameter, 0, "@URL|")){
			auto urlParameter = context.parameters.urlParameters.find(name);
			if(urlParameter == context.parameters.urlParameters.end()){
				chopFront(parameter, 5);
			}
			else{
				parameter = urlParameter->second;
			}
		}
	}
}

namespace Parser{
	
    void handleListPages(TreeContext& context, const Token& token){
		auto db = context.parameters.database;
        if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"ListPages failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
        
        if(context.parameters.includeDepth > 5){//if we have gone too many layers deep, then we need to stop doing this
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"ListPages failed: maximum inline depth reached"}});
            makeDivPushable(context);
            return;
        }
        
        const SectionComplete& section = std::get<SectionComplete>(token.token);
        
        Database::AdvancedPageQueryParameters parameters;
        {
        	Database::AdvancedPageQueryParameters::CategorySelector categorySelect;
        	
        	auto rawParameter = section.parameters.find("category");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "category", context);
				if(parameter == "*" || parameter == ""){
					//don't do anything here
				}
				else if(parameter == "."){
					categorySelect.included.push_back(getPageCategory(context.parameters.page.name));
					parameters.categorySelect = categorySelect;
				}
				else{
					std::vector<std::string> categories = splitString(parameter);
					if(categories.size() > 0){
						for(const std::string& category : categories){
							if(check(category, 0, "-")){
								std::string temp = category;
								chopFront(temp, 1);
								categorySelect.excluded.push_back(temp);
							}
							else{
								categorySelect.included.push_back(category);
							}
						}
						parameters.categorySelect = categorySelect;
					}
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::TagSelector tagSelect;
        	
        	auto rawParameter = section.parameters.find("tags");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "tags", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "-"){
					tagSelect.noTags = true;
					parameters.tagSelect = tagSelect;
				}
				else if(parameter == "="){
					tagSelect.noTags = false;
					tagSelect.included = context.parameters.page.tags;
					parameters.tagSelect = tagSelect;
				}
				else if(parameter == "=="){
					tagSelect.noTags = false;
					tagSelect.mustIncluded = context.parameters.page.tags;
					parameters.tagSelect = tagSelect;
				}
				else{
					std::vector<std::string> tags = splitString(parameter);
					if(tags.size() > 0){
						for(const std::string& tag : tags){
							if(check(tag, 0, "-")){
								std::string temp = tag;
								chopFront(temp, 1);
								tagSelect.excluded.push_back(temp);
							}
							else if(check(tag, 0, "+")){
								std::string temp = tag;
								chopFront(temp, 1);
								tagSelect.mustIncluded.push_back(temp);
							}
							else{
								tagSelect.included.push_back(tag);
							}
						}
						tagSelect.noTags = false;
						parameters.tagSelect = tagSelect;
					}
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::ParentSelector parentSelect;
        	
        	auto rawParameter = section.parameters.find("parent");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "parent", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "-"){
					parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::NoParent;
					parameters.parentSelect = parentSelect;
				}
				else if(parameter == "="){
					parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::WithParent;
					if(context.parameters.page.parent){
						parentSelect.parent = *context.parameters.page.parent;
						parameters.parentSelect = parentSelect;
					}
					else{
						parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::NoParent;
						parameters.parentSelect = parentSelect;
					}
				}
				else if(parameter == "-="){
					parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::WithoutParent;
					if(context.parameters.page.parent){
						parentSelect.parent = *context.parameters.page.parent;
						parameters.parentSelect = parentSelect;
					}
				}
				else if(parameter == "."){
					parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::WithParent;
					parentSelect.parent = context.parameters.page.name;
					parameters.parentSelect = parentSelect;
				}
				else{
					parentSelect.type = Database::AdvancedPageQueryParameters::ParentSelector::WithParent;
					parentSelect.parent = parameter;
					parameters.parentSelect = parentSelect;
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::DateSelector dateSelect;
        	
        	auto rawParameter = section.parameters.find("created_at");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "created_at", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "="){
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::WithinDay;
					dateSelect.time = context.parameters.page.creationTimeStamp;
					parameters.dateSelect = dateSelect;
				}
				else if(check(parameter, 0, "<=")){
					chopFront(parameter, 2);
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::LessEqual;
					if(parseDate(parameter, dateSelect.time) != DateCode::Error){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, ">=")){
					chopFront(parameter, 2);
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::GreaterEqual;
					if(parseDate(parameter, dateSelect.time) != DateCode::Error){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, "<>")){
					chopFront(parameter, 2);
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::NotEqual;
					if(parseDate(parameter, dateSelect.time) != DateCode::Error){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, "<")){
					chopFront(parameter, 1);
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::Less;
					if(parseDate(parameter, dateSelect.time) != DateCode::Error){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, ">")){
					chopFront(parameter, 1);
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::Greater;
					if(parseDate(parameter, dateSelect.time) != DateCode::Error){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, "last")){
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::GreaterEqual;
					if(parseDateRangeNumber(parameter, dateSelect.time)){
						parameters.dateSelect = dateSelect;
					}
				}
				else if(check(parameter, 0, "older")){
					dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::Less;
					if(parseDateRangeNumber(parameter, dateSelect.time)){
						parameters.dateSelect = dateSelect;
					}
				}
				else{
					if(check(parameter, 0, "=")){
						chopFront(parameter, 1);
					}
					switch(parseDate(parameter, dateSelect.time)){
						case DateCode::Error:
							//do nothing!
							break;
						case DateCode::Month:
							dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::WithinMonth;
							dateSelect.time += (60*60*24*30) / 2;
							parameters.dateSelect = dateSelect;
							break;
						case DateCode::Year:
							dateSelect.type = Database::AdvancedPageQueryParameters::DateSelector::WithinYear;
							dateSelect.time += (60*60*24*365) / 2;
							parameters.dateSelect = dateSelect;
							break;
					}
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::AuthorSelector authorSelect;
        	
        	auto rawParameter = section.parameters.find("created_by");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "created_by", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "="){
					authorSelect.type = Database::AdvancedPageQueryParameters::AuthorSelector::Include;
					if(context.parameters.page.author){
						authorSelect.author = *context.parameters.page.author;
						parameters.authorSelect = authorSelect;
					}
				}
				else if(parameter == "-="){
					authorSelect.type = Database::AdvancedPageQueryParameters::AuthorSelector::Exclude;
					if(context.parameters.page.author){
						authorSelect.author = *context.parameters.page.author;
						parameters.authorSelect = authorSelect;
					}
				}
				else{
					authorSelect.type = Database::AdvancedPageQueryParameters::AuthorSelector::Include;
					authorSelect.author = parameter;
					parameters.authorSelect = authorSelect;
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::RatingSelector ratingSelect;
        	
        	auto rawParameter = section.parameters.find("rating");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "rating", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "="){
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::Equal;
					ratingSelect.rating = context.parameters.page.rating;
					parameters.ratingSelect = ratingSelect;
				}
				else if(check(parameter, 0, "<=")){
					chopFront(parameter, 2);
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::LessEqual;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, ">=")){
					chopFront(parameter, 2);
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::GreaterEqual;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, "<>")){
					chopFront(parameter, 2);
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::NotEqual;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, "<")){
					chopFront(parameter, 1);
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::Less;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, ">")){
					chopFront(parameter, 1);
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::Greater;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else{
					if(check(parameter, 0, "=")){
						chopFront(parameter, 1);
					}
					ratingSelect.type = Database::AdvancedPageQueryParameters::RatingSelector::Equal;
					try{
						ratingSelect.rating = std::stoll(parameter);
						parameters.ratingSelect = ratingSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::VoteSelector voteSelect;
        	
        	auto rawParameter = section.parameters.find("votes");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "votes", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "="){
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::Equal;
					voteSelect.votes = context.parameters.page.votes;
					parameters.voteSelect = voteSelect;
				}
				else if(check(parameter, 0, "<=")){
					chopFront(parameter, 2);
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::LessEqual;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, ">=")){
					chopFront(parameter, 2);
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::GreaterEqual;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, "<>")){
					chopFront(parameter, 2);
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::NotEqual;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, "<")){
					chopFront(parameter, 1);
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::Less;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else if(check(parameter, 0, ">")){
					chopFront(parameter, 1);
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::Greater;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
				else{
					if(check(parameter, 0, "=")){
						chopFront(parameter, 1);
					}
					voteSelect.type = Database::AdvancedPageQueryParameters::VoteSelector::Equal;
					try{
						voteSelect.votes = std::stoll(parameter);
						parameters.voteSelect = voteSelect;
					}
					catch(std::exception& e){
						//ignore errors
					}
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::NameSelector nameSelect;
        	
        	auto rawParameter = section.parameters.find("name");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "name", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "="){
					nameSelect.type = Database::AdvancedPageQueryParameters::NameSelector::Name;
					nameSelect.name = getPageName(context.parameters.page.name);
					parameters.nameSelect = nameSelect;
				}
				else if(parameter[parameter.size() - 1] == '%' || parameter[parameter.size() - 1] == '*'){
					nameSelect.type = Database::AdvancedPageQueryParameters::NameSelector::Starting;
					nameSelect.name = parameter.substr(0, parameter.size() - 1);
					parameters.nameSelect = nameSelect;
				}
				else{
					nameSelect.type = Database::AdvancedPageQueryParameters::NameSelector::Name;
					nameSelect.name = parameter;
					parameters.nameSelect = nameSelect;
				}
        	}
        }
        {
			Database::AdvancedPageQueryParameters::FullNameSelector fullNameSelect;
			
			auto rawParameter = section.parameters.find("fullname");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "fullname", context);
				if(parameter == ""){
					//don't do anything here
				}
				else{
					fullNameSelect.fullName = parameter;
					parameters.fullNameSelect = fullNameSelect;
				}
        	}
        }
        {
        	Database::AdvancedPageQueryParameters::RangeSelector rangeSelect;
        	
        	auto rawParameter = section.parameters.find("range");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "range", context);
				if(parameter == ""){
					//don't do anything here
				}
				else if(parameter == "."){
					rangeSelect.page = context.parameters.page.name;
					rangeSelect.type = Database::AdvancedPageQueryParameters::RangeSelector::Only;
					parameters.rangeSelect = rangeSelect;
				}
				else if(parameter == "before"){
					rangeSelect.page = context.parameters.page.name;
					rangeSelect.type = Database::AdvancedPageQueryParameters::RangeSelector::Before;
					parameters.rangeSelect = rangeSelect;
				}
				else if(parameter == "after"){
					rangeSelect.page = context.parameters.page.name;
					rangeSelect.type = Database::AdvancedPageQueryParameters::RangeSelector::After;
					parameters.rangeSelect = rangeSelect;
				}
				else if(parameter == "others"){
					rangeSelect.page = context.parameters.page.name;
					rangeSelect.type = Database::AdvancedPageQueryParameters::RangeSelector::Others;
					parameters.rangeSelect = rangeSelect;
				}
        	}
        }
        {
			Database::AdvancedPageQueryParameters::Ordering ordering;
			ordering.value = Database::AdvancedPageQueryParameters::Ordering::CreatedTime;
			ordering.order = Database::AdvancedPageQueryParameters::Ordering::Descending;
			
			auto rawParameter = section.parameters.find("order");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "order", context);
				if(parameter == ""){
					//don't do anything here
				}
				else{
					std::vector<std::string> orderSpec = splitString(parameter);
					bool matched = false;
					if(orderSpec.size() == 1){
						matched = true;
						ordering.order = Database::AdvancedPageQueryParameters::Ordering::Ascending;
					}
					else if(orderSpec.size() == 2 && orderSpec[1] == "desc"){
						matched = true;
						ordering.order = Database::AdvancedPageQueryParameters::Ordering::Descending;
					}
					else if(orderSpec.size() == 3 && orderSpec[1] == "desc" && orderSpec[2] == "desc"){
						matched = true;
						ordering.order = Database::AdvancedPageQueryParameters::Ordering::Ascending;
					}
					
					if(matched){
						std::string value = orderSpec[0];
						if(value == "name"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Name;
						}
						else if(value == "fullname"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::FullName;
						}
						else if(value == "title"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Title;
						}
						else if(value == "created_by"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Creator;
						}
						else if(value == "created_at"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::CreatedTime;
						}
						else if(value == "updated_at"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::UpdatedTime;
						}
						else if(value == "size"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Size;
						}
						else if(value == "rating"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Rating;
						}
						else if(value == "votes"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Votes;
						}
						else if(value == "revisions"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Revisions;
						}
						else if(value == "comments"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Comments;
						}
						else if(value == "random"){
							ordering.value = Database::AdvancedPageQueryParameters::Ordering::Random;
						}
					}
				}
        	}
			
			parameters.ordering = ordering;
        }
        {
			std::int64_t limit = 20;
			
			auto rawParameter = section.parameters.find("limit");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				handleUrlParameter(parameter, "limit", context);
				if(parameter == ""){
					//don't do anything here
				}
				else{
					try{
						limit = std::stoull(parameter);
					}
					catch(std::exception& e){
						//if there's an error, just do nothing
					}
				}
        	}
        	
        	parameters.limit = std::min<std::int64_t>(limit, 250);//maximum of 250(is that enough?)
        }
        {
			std::int64_t offset = 0;
			
			auto rawParameter = section.parameters.find("offset");
        	if(rawParameter != section.parameters.end()){
				std::string parameter = rawParameter->second;
				trimString(parameter);
				std::cout << "OFFSET: " << parameter << " -> ";
				handleUrlParameter(parameter, "offset", context);
				std::cout << parameter << "\n";
				if(parameter == ""){
					//don't do anything here
				}
				else{
					try{
						offset = std::stoull(parameter);
					}
					catch(std::exception& e){
						//if there's an error, just do nothing
					}
				}
        	}
        	
        	parameters.offset = offset;
        }
        
        std::vector<Database::ID> pageList = db->advancedPageQuery(parameters);
        
        std::string prependLine;
		{
			auto rawParameter = section.parameters.find("prependLine");
        	if(rawParameter != section.parameters.end()){
				prependLine = rawParameter->second;
        	}
		}
        std::string appendLine;
        {
			auto rawParameter = section.parameters.find("appendLine");
        	if(rawParameter != section.parameters.end()){
				appendLine = rawParameter->second;
        	}
		}
        bool wrapper = true;
        {
			auto rawParameter = section.parameters.find("wrapper");
        	if(rawParameter != section.parameters.end() && rawParameter->second == "no"){
				wrapper = false;
        	}
        }
        bool separate = true;
        {
			auto rawParameter = section.parameters.find("separate");
        	if(rawParameter != section.parameters.end() && rawParameter->second == "no"){
				separate = false;
        	}
        }
        
        std::vector<Node> content;
        
        if(separate){
			for(Database::ID pageId : pageList){
				Database::PageRevision pageRevision = db->getLatestPageRevision(pageId);
				PageInfo pageInfo = getPageInfo(db, pageId);
				pageRevision.sourceCode = applyPageTemplate(section.contents, pageRevision.sourceCode, pageInfo);
				ParserParameters parameters = context.parameters;
				parameters.includeDepth = context.parameters.includeDepth + 1;
				parameters.page = pageInfo;
				PageTree pageTree = makeTreeFromPage(pageRevision.sourceCode, parameters);
				//get all of the nodes from the pageTree and append them to content
				content.insert(content.end(), pageTree.pageRoot.branches.begin(), pageTree.pageRoot.branches.end());
			}
        }
        else{
			std::string sourceCode = prependLine + "\n";
			for(Database::ID pageId : pageList){
				Database::PageRevision pageRevision = db->getLatestPageRevision(pageId);
				PageInfo pageInfo = getPageInfo(db, pageId);
				sourceCode += applyPageTemplate(section.contents, pageRevision.sourceCode, pageInfo);
			}
			sourceCode += appendLine;
			
			ParserParameters parameters = context.parameters;
			parameters.includeDepth = context.parameters.includeDepth + 1;
			PageTree pageTree = makeTreeFromPage(sourceCode, parameters);
			content = pageTree.pageRoot.branches;
        }
        
        if(wrapper){
			addAsDiv(context, Node{Div{}, content});
        }
        else{
			makeDivPushable(context);
			//get all of the nodes from content and append them into the current page
			context.stack.back().branches.insert(context.stack.back().branches.end(), content.begin(), content.end());
        }
    }
}
