#include "Parser.h"

#include "Scraper.h"//needed to download images

Token::Token(Type _type, std::string _sectionType, std::string _sectionName, std::string _data, int _numData):
	type(_type),sectionType(_sectionType),sectionName(_sectionName),data(_data),numData(_numData){
}

void Statement::print(int tab){
	for(int i = 0; i < tab; i++){
		std::cout << "\t";
	}
	switch(type){
	case Type::Article:
		std::cout << "Article:\n";
		break;
	case Type::TabSection:
		std::cout << "TabSection:" << extraDataA << "\n";
		break;
	case Type::Collapsible:
		std::cout << "Collapsible:\n";
		break;
	case Type::Quote:
		std::cout << "Quote:\n";
		break;
	case Type::Header:
		std::cout << "Header:\n";
		break;
	case Type::FootnoteBlock:
		std::cout << "FootnoteBlock:\n";
		break;
	case Type::HtmlBlock:
		std::cout << "HtmlBlock:\n";
		break;
	case Type::ImageBlock:
		std::cout << "ImageBlock:\n";
		break;
	case Type::Image:
		std::cout << "Image:\n";
		break;
	case Type::PageDivider:
		std::cout << "PageDivider:\n";
		break;
	case Type::PageClearer:
		std::cout << "PageClearer:\n";
		break;
	case Type::Paragraph:
		std::cout << "Paragraph:\n";
		break;
	case Type::TabView:
		std::cout << "TabView:\n";
		break;
	case Type::OrderedList:
		std::cout << "OrderedList:\n";
		break;
	case Type::BulletList:
		std::cout << "BulletList:\n";
		break;
	case Type::Table:
		std::cout << "Table:\n";
		break;
	case Type::TableRow:
		std::cout << "TableRow:\n";
		break;
	case Type::Code:
		std::cout << "Code: " << extraDataA << "\n";
		break;
	case Type::Div:
		std::cout << "Div:\n";
		break;
	case Type::CenterAlign:
		std::cout << "Center:\n";
		break;
	case Type::JustifyAlign:
		std::cout << "JustifyAlign:\n";
		break;
	case Type::RightAlign:
		std::cout << "RightAlign:\n";
		break;
	case Type::LeftAlign:
		std::cout << "LeftAlign:\n";
		break;
	case Type::LineBreak:
		std::cout << "LineBreak:\n";
		break;
	case Type::Error:
		std::cout << "Error: " << extraDataA << "\n";
		break;
	case Type::Text:
		std::cout << "Text: \"" << extraDataA << "\"\n";
		break;
	case Type::ListElement:
		std::cout << "ListElement:\n";
		break;
	case Type::TableElement:
		std::cout << "TableElement: " << extraDataA << " | " << extraDataB << "\n";
		break;
	case Type::Footnote:
		std::cout << "Footnote:\n";
		break;
	case Type::Span:
		std::cout << "Span:\n";
		break;
	case Type::Color:
		std::cout << "Color: " << extraDataA << "\n";
		break;
	case Type::Size:
		std::cout << "Size: " << extraDataA << "\n";
		break;
	case Type::Italic:
		std::cout << "Italic:\n";
		break;
	case Type::Bold:
		std::cout << "Bold:\n";
		break;
	case Type::Underline:
		std::cout << "Underline:\n";
		break;
	case Type::Strikethrough:
		std::cout << "Strikethrough:\n";
		break;
	case Type::Monospaced:
		std::cout << "Monospaced:\n";
		break;
	case Type::Superscript:
		std::cout << "Superscript:\n";
		break;
	case Type::Subscript:
		std::cout << "Subscript:\n";
		break;
	case Type::HyperLink:
		std::cout << "HyperLink: " << extraDataA << "<-" << extraDataB << "\n";
		break;
	case Type::User:
		std::cout << "User:" << extraDataA << "\n";
		break;
	default:
		throw std::runtime_error("Attempted To Print Invalid Statement");
	}
	
	for(auto i = statements.begin(); i != statements.end(); i++){
		i->print(tab + 1);
	}
}

namespace{
	std::string escapeHtml(std::string text){
		replaceAll(text, "&", "&amp;");
		replaceAll(text, "\"", "&quot;");
		replaceAll(text, "<", "&lt;");
		replaceAll(text, ">", "&gt;");
		return text;
	}
}

void Statement::toRawHtml(std::string& output, ParserConvertData& convertData){
	switch(type){
	case Type::Article:
		output += "<h1>" + extraDataA + "</h1><hr>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		break;
	case Type::TabSection:
		if(convertData.printerFriendly){
			output +=
			"<h2>"
			+ extraDataA +
			"</h2>";
			for(auto i = statements.begin(); i != statements.end(); i++){
				i->toRawHtml(output, convertData);
			}
		}
		else{
			output +=
			"<div class='TabSection'>"
			"<div class='TabSectionTitle'>"
			+ extraDataA +
			"</div>";
			for(auto i = statements.begin(); i != statements.end(); i++){
				i->toRawHtml(output, convertData);
			}
			output += 
			"</div>";
		}
		break;
	case Type::Collapsible:
		output += 
		"<div class='CollapsibleContainer'>"
		"<input type='checkbox' id='button-" + std::to_string(convertData.uniqueNumber) + "'>"
		"<label class='CollapsibleHiddenText' for='button-" + std::to_string(convertData.uniqueNumber) +  "'>"
		+ extraDataA +
		"</label>"
		"<label class='CollapsibleShownText' for='button-" + std::to_string(convertData.uniqueNumber) +  "'>"
		+ extraDataB +
		"</label>"
		"<div class='CollapsibleContent'>";
		
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		
		output += "</div>"
		"</div>";
		convertData.uniqueNumber++;
		break;
	case Type::Quote:
		output += "<blockquote>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</blockquote>";
		break;
	case Type::Div:
		output += "<div " + extraDataA + ">";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</div>";
		break;
	case Type::CenterAlign:
		output += "<div class='CenterAlign'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</div>";
		break;
	case Type::JustifyAlign:
		output += "<div class='JustifyAlign'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</div>";
		break;
	case Type::RightAlign:
		output += "<div class='RightAlign'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</div>";
		break;
	case Type::LeftAlign:
		output += "<div class='LeftAlign'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</div>";
		break;
	case Type::Header:
		output += "<h" + extraDataA + ">";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</h" + extraDataA + ">";
		break;
	case Type::FootnoteBlock:
		if(convertData.footnotes.size() > 0){
			output +=
			"<div class='FootnoteBlock'>"
			"<h2>"
			"Footnotes"
			"</h2>"
			"<ol>";
			for(auto i = convertData.footnotes.begin(); i != convertData.footnotes.end(); i++){
				output += "<li>";
				for(auto y = i->statements.begin(); y != i->statements.end(); y++){
					y->toRawHtml(output, convertData);
				}
				output += "</li>";
			}
			output +=
			"</ol>"
			"</div>";
			convertData.footnotes.clear();
		}
		break;
	case Type::HtmlBlock:
		output += extraDataA;
		break;
	case Type::ImageBlock:
		{
			std::string encoded = percentEncode(extraDataA);//yes, I mean to double encode this string
			Scraper::downloadImage(extraDataA, convertData);
			output += 
			"<div class='ImageBlock' " + ((extraDataB=="")?std::string(""):("style='width:" + extraDataB + ";'")) + ">"
			"<img src='" + percentEncode(encoded) + "' alt='"+ escapeHtml(extraDataA) + "'>"
			"<div class='ImageBlockCaption'>";
			for(auto i = statements.begin(); i != statements.end(); i++){
				i->toRawHtml(output, convertData);
			}
			output += "</div>"
			"</div>";
		}
		break;
	case Type::Image:
		{
			std::string encoded = percentEncode(extraDataA);//yes, I mean to double encode this string
			Scraper::downloadImage(extraDataA, convertData);
			if(extraDataC == "block"){
				output += "<div class='ImageContainer'>";
			}
			output += 
			"<img src='" + percentEncode(encoded) + "' alt='"+ extraDataA + "' " + extraDataB + ">";
			if(extraDataB == "block"){
				output += "</div>";
			}
		}
		break;
	case Type::PageDivider:
		output += "<hr>";
		break;
	case Type::PageClearer:
		output += "<div class='PageClearer'></div>";
		break;
	case Type::Paragraph:
		output += "<p>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</p>";
		break;
	case Type::TabView:
		if(convertData.printerFriendly){
			for(auto i = statements.begin(); i != statements.end(); i++){
				i->toRawHtml(output, convertData);
			}
		}
		else{
			output +=
			"<div class='TabView'>"
			"<div class='TabViewButtons'>";
			std::string formId = "form-" + std::to_string(convertData.uniqueNumber);
			convertData.uniqueNumber++;
			output += "<form id='" + formId + "'></form>";
			std::size_t radioId = convertData.uniqueNumber;
			convertData.uniqueNumber += statements.size();
			{
				int num = 0;
				for(auto i = statements.begin(); i != statements.end(); i++){
					output += "<label class='TabViewButton' for='button-" + std::to_string(radioId + num) + "'>" + i->extraDataA + "</label>";
					num++;
				}
			}
			output += "</div>";
			{
				int num = 0;
				for(auto i = statements.begin(); i != statements.end(); i++){
					output += "<input type='radio' name='" + formId + "' id='button-" + std::to_string(radioId + num) + "'" + (num==0?" checked":"") + ">";
					i->toRawHtml(output, convertData);
					num++;
				}
			}
			output += "</div>";
		}
		break;
	case Type::OrderedList:
		output += "<ol>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</ol>";
		break;
	case Type::BulletList:
		output += "<ul>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</ul>";
		break;
	case Type::Table:
		output += "<table>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</table>";
		break;
	case Type::TableRow:
		output += "<tr>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</tr>";
		break;
	case Type::Code:
		{
			std::string temp = escapeHtml(extraDataA);
			replaceAll(temp, "\n", "<br>");
			output += "<code>" + temp + "</code>\n";
		}
		break;
	case Type::LineBreak:
		output += "<br>";
		break;
	case Type::Error:
		if(!convertData.printerFriendly){
			output += escapeHtml(extraDataA);
		}
		break; 
	case Type::Text:
		if(extraDataB == "escapeWhiteSpace"){
			std::string temp = escapeHtml(extraDataA);
			replaceAll(temp, " ", "&nbsp;");
			output += temp;
		}
		else{
			output += escapeHtml(extraDataA);
		}
		break; 
	case Type::ListElement:
		output += "<li>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</li>";
		break;
	case Type::TableElement:
		if(extraDataA == "~"){
			output += "<td " + ((extraDataB == "1")?(std::string()):("colspan='" + extraDataB + "'")) + ">";
		}
		else if(extraDataA == "<"){
			output += "<td class='TableElementLeftAlign' " + ((extraDataB == "1")?(std::string()):("colspan='" + extraDataB + "'")) + ">";
		}
		else if(extraDataA == ">"){
			output += "<td class='TableElementRightAlign' " + ((extraDataB == "1")?(std::string()):("colspan='" + extraDataB + "'")) + ">";
		}
		else if(extraDataA == "="){
			output += "<td class='TableElementCenterAlign' " + ((extraDataB == "1")?(std::string()):("colspan='" + extraDataB + "'")) + ">";
		}
		else if(extraDataA == ""){
			output += "<td " + ((extraDataB == "1")?(std::string()):("colspan='" + extraDataB + "'")) + ">";
		}
		else{
			throw std::runtime_error("Error on table formatting");
		}
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		if(extraDataA == "~"){
			output += "</td>";
		}
		else{
			output += "</td>";
		}
		break;
	case Type::Footnote:
		convertData.footnotes.push_back(*this);
		output += "<sup>" + std::to_string(convertData.footnotes.size()) + "</sup>";
		break;
	case Type::Span:
		output += "<span " + extraDataA + ">";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Color:
		std::transform(extraDataA.begin(), extraDataA.end(), extraDataA.begin(), ::tolower);
		if(extraDataA == "purple" || extraDataA == "silver" || extraDataA.size() != 6){
			output += "<span style='color:" + extraDataA + ";'>";
		}
		else{
			output += "<span style='color:#" + extraDataA + ";'>";
		}
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Size:
		output += "<span style='font-size:" + extraDataA + ";'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Italic:
		output += "<em>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</em>";
		break;
	case Type::Bold:
		output += "<strong>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</strong>";
		break;
	case Type::Underline:
		output += "<span class='Underline'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Strikethrough:
		output += "<span class='Strikethrough'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Monospaced:
		output += "<span class='Monospaced'>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</span>";
		break;
	case Type::Superscript:
		output += "<sup>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</sup>";
		break;
	case Type::Subscript:
		output += "<sub>";
		for(auto i = statements.begin(); i != statements.end(); i++){
			i->toRawHtml(output, convertData);
		}
		output += "</sub>";
		break;
	case Type::HyperLink:
		output += "<a href='" + extraDataA + "'>" + escapeHtml(extraDataB) + "</a>";
		break;
	case Type::User:
		{
			std::string link = extraDataA;
			replaceAll(link, " ", "-");
			std::transform(link.begin(), link.end(), link.begin(), ::tolower);
			output += "<a href='user:info/" + link + "'>" + escapeHtml(extraDataA) + "</a>";
		}
		break;
	}
}

namespace{
	bool check(std::string& buffer, std::size_t pos, std::string text){
		if(pos + text.size() > buffer.size()){
			return false;
		}
		std::string temp = buffer.substr(pos, text.size());
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		return text == temp;
	}

	void emptyBuffer(std::string& buffer, std::vector<Token>& output){
		if(buffer.size() > 0){
			output.push_back(Token(Token::Type::PlainText, "", "", buffer));
		}
		buffer.clear();
	}
	
	void checkIsTable(std::string& article, std::size_t pos, bool& isTable, bool& isEnd){
		isEnd = true;
		for(std::size_t i = 2; i + pos < article.size(); i++){
			if(isEnd){
				if(check(article, i + pos, "\n")){
					isTable = true;
					break;
				}
			}
			else if(check(article, i + pos, "\n\n")){
				break;
			}
			
			if(check(article, i + pos, "||")){
				isEnd = false;
				{
					bool isEndTemp;
					checkIsTable(article, i + pos, isTable, isEndTemp);
				}
				break;
			}
			if(article[i + pos] != ' '){
				isEnd = false;
			}
		}
	}
}

std::vector<Token> Parser::tokenizeArticle(std::string& article){
	std::vector<Token> output;
	
	std::size_t pos = 0;
	bool isNewLine = true;
	bool overrideNewLine = true;//first line should work as if it is after a newline, so default is enabled
	//set true when a character appears that isn't a new line, but it needs the next cycle of the loop to ignore the character making it not a newline, and treat it like a newline anyways
	std::string buffer;
	
	bool isStrikeThrough = false;
	bool isBold = false;
	bool isUnderLine = false;
	bool isColored = false;
	
	for(; pos < article.size();){
		isNewLine = false;
		if(overrideNewLine){
			isNewLine = true;
			overrideNewLine = false;
		}
		if(check(article, pos, "\n")){
			isStrikeThrough = false;
			isBold = false;
			isUnderLine = false;
			isColored = false;
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::NewLine, "", "", ""));
			isNewLine = true;
			pos += 1;
			if(check(article, pos, "\n")){
				continue;
			}
		}
		if(check(article, pos, "_\n")){
			pos += 2;
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::NewLine, "", "", ""));
			continue;
		}
		if(check(article, pos, " _ ")){
			isStrikeThrough = false;
			isBold = false;
			isUnderLine = false;
			isColored = false;
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::NewLine, "", "", ""));
			isNewLine = true;
			pos += 3;
			if(check(article, pos, "\n")){
				continue;
			}
		}
		
		/*
		if(check(article, pos, "\\")){
			pos += 2;
			continue;
		}
		*/
		
		if(check(article, pos, "http://")){
			buffer += "http://";
			pos += 7;
			continue;
		}
		
		if(check(article, pos, "https://")){
			buffer += "https://";
			pos += 8;
			continue;
		}
		
		if(check(article, pos, "----")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::SectionComplete, "pageDivider", "", ""));
			pos += 4;
			while(pos < article.size() && article[pos] == '-'){
				pos++;
			}
			continue;
		}
		
		if(check(article, pos, "~~~~")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::SectionComplete, "pageClearer", "", ""));
			pos += 4;
			while(pos < article.size() && article[pos] == '~'){
				pos++;
			}
			continue;
		}
		
		if(check(article, pos, "//")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::FormatMarker, "italic", "", ""));
			pos += 2;
			continue;
		}
		
		if(check(article, pos, "**")){
			bool isBoldToken = false;
			if(isBold){
				isBoldToken = true;
			}
			else{
				for(int i = 2; i < pos + article.size(); i++){
					if(check(article, i + pos, "\n")){
						break;
					}
					if(check(article, i + pos, "_\n")){
						i++;
						continue;
					}
					if(check(article, i + pos, "**")){
						isBoldToken = true;
						break;
					}
				}
			}
			if(isBoldToken){
				isBold = !isBold;
				emptyBuffer(buffer, output);
				output.push_back(Token(Token::Type::FormatMarker, "bold", "", ""));
				pos += 2;
				continue;
			}
		}
		
		if(check(article, pos, "__")){
			bool isUnderLineToken = false;
			if(isUnderLine){
				isUnderLineToken = true;
			}
			else{
				for(int i = 2; i < pos + article.size(); i++){
					if(check(article, i + pos, "\n")){
						break;
					}
					if(check(article, i + pos, "_\n")){
						i++;
						continue;
					}
					if(check(article, i + pos, "__")){
						isUnderLineToken = true;
						break;
					}
				}
			}
			if(isUnderLineToken){
				isUnderLine = !isUnderLine;
				emptyBuffer(buffer, output);
				output.push_back(Token(Token::Type::FormatMarker, "underline", "", ""));
				pos += 2;
				continue;
			}
		}
		
		if(!isStrikeThrough && check(article, pos, " -- ")){
			buffer += u8" —";
			pos += 3;
			continue;
		}
		
		if(check(article, pos, "--")){
			bool isDash = true;
			if(isStrikeThrough){
				isDash = false;
			}
			else{
				for(int i = 2; i < pos + article.size(); i++){
					if(check(article, i + pos, "\n")){
						break;
					}
					if(check(article, i + pos, "_\n")){
						i++;
						continue;
					}
					if(check(article, i + pos, " -- ")){
						i += 3;
						continue;
					}
					if(check(article, i + pos, "--")){
						isDash = false;
						break;
					}
				}
			}
			if(!isDash){
				emptyBuffer(buffer, output);
				output.push_back(Token(Token::Type::FormatMarker, "strikethrough", "", ""));
				pos += 2;
				isStrikeThrough = !isStrikeThrough;
				continue;
			}
			buffer += u8"—";
			pos += 2;
			continue;
		}
		
		if(check(article, pos, "##")){
			if(isColored){
				emptyBuffer(buffer, output);
				pos += 2;
				output.push_back(Token(Token::Type::FormatMarker, "color", "", ""));
				isColored = false;
			}
			else{
				pos += 2;
				std::size_t endPos = pos;
				std::string internal;
				for(; endPos < article.size(); endPos++){
					if(check(article, endPos, "##")){
						internal = article.substr(pos, endPos - pos);
						break;
					}
					if(check(article, endPos, "_\n")){
						endPos++;
						continue;
					}
					if(check(article, endPos, "\n")){
						break;
					}
				}
				if(internal != ""){
					emptyBuffer(buffer, output);
					std::size_t endColorPos = internal.find("|");
					std::string color = internal.substr(0, endColorPos);
					output.push_back(Token(Token::Type::FormatMarker, "color", "", color));
					pos += endColorPos + 1;
					isColored = true;
				}
				else{
					buffer += "##";
				}
			}
			continue;
		}
		
		if(check(article, pos, "{{")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::FormatMarker, "monospaced", "", ""));
			pos += 2;
			continue;
		}
		
		if(check(article, pos, "}}")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::FormatMarker, "monospaced", "", ""));
			pos += 2;
			continue;
		}
		
		if(check(article, pos, "^^")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::FormatMarker, "superscript", "", ""));
			pos += 2;
			continue;
		}
		
		if(check(article, pos, ",,")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::FormatMarker, "subscript", "", ""));
			pos += 2;
			continue;
		}
		
		if(isNewLine && check(article, pos, ">")){//check for quoteblock markers
			if(output.back().type == Token::Type::PrefixFormat && output.back().sectionType == "quoteBlock"){
				buffer += ">";
				pos++;
				continue;
			}
			else{
				std::size_t length = 0;
				bool completed = false;
				while(pos + length < article.size()){
					if(article[pos + length] == '>'){
						length++;
					}
					else if(article[pos + length] == ' '){
						pos += length + 1;
						completed = true;
						break;
					}
					else if(article[pos + length] == '\n'){
						pos += length;
						completed = true;
						break;
					}
					else{
						break;
					}
				}
				if(article[pos] == '\n' && article[pos - 1] == '>'){//experimental, probably safe to remove
					pos++;
					overrideNewLine = true;
					continue;
				}
				if(completed){
					emptyBuffer(buffer, output);
					output.push_back(Token(Token::Type::PrefixFormat, "quoteBlock", "", "", length));
					//things that appear after this one should still think they are right after a newline
					overrideNewLine = true;
					continue;
				}
			}
		}
		
		if(isNewLine && check(article, pos, "* ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "listBullet", "", ""));
			pos += 2;
			continue;
		}
		
		if(check(article, pos, "||")){
			bool isTable = false;
			bool isEnd;
			checkIsTable(article, pos, isTable, isEnd);
			if(isTable){
				if(isEnd){
					emptyBuffer(buffer, output);
					output.push_back(Token(Token::Type::PrefixFormat, "tableRowEndMarker", "", ""));
					while(article[pos] != '\n'){
						pos++;
					}
					continue;
				}
				else{
					emptyBuffer(buffer, output);
					std::string formatting;
					int length = 0;
					while(check(article, pos, "||")){
						pos += 2;
						length++;
					}
					if(pos >= article.size()){
						continue;
					}
					if(article[pos] == '~'){
						formatting = '~';
						pos++;
					}
					else if(article[pos] == '<'){
						formatting = '<';
						pos++;
					}
					else if(article[pos] == '>'){
						formatting = '>';
						pos++;
					}
					else if(article[pos] == '='){
						formatting = '=';
						pos++;
					}
					output.push_back(Token(Token::Type::PrefixFormat, "tableMarker", formatting, std::to_string(length)));
					continue;
				}
			}
		}
		
		if(check(article, pos, "\\") && check(article, pos + 1, "\n")){// '\' at the end of lines means to ignore the newline
			pos += 2;
			continue;
		}
		
		if(isNewLine && check(article, pos, "# ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "listNumber", "", ""));
			pos += 2;
			continue;
		}
		
		if(isNewLine && check(article, pos, "+ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "1"));
			pos += 2;
			continue;
		}
		
		if(isNewLine && check(article, pos, "++ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "2"));
			pos += 3;
			continue;
		}
		
		if(isNewLine && check(article, pos, "+++ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "3"));
			pos += 4;
			continue;
		}
		
		if(isNewLine && check(article, pos, "++++ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "4"));
			pos += 5;
			continue;
		}
		
		if(isNewLine && check(article, pos, "+++++ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "5"));
			pos += 6;
			continue;
		}
		
		if(isNewLine && check(article, pos, "++++++ ")){
			emptyBuffer(buffer, output);
			output.push_back(Token(Token::Type::PrefixFormat, "header", "", "6"));
			pos += 7;
			continue;
		}
		
		if(check(article, pos, "@@")){
			pos += 2;
			std::size_t escapeStart = pos;
			for(; pos < article.size(); pos++){
				if(check(article, pos, "@@")){
					emptyBuffer(buffer, output);
					std::string text = article.substr(escapeStart, pos - escapeStart);
					output.push_back(Token(Token::Type::PlainText, "escapeWhiteSpace", "", text));
					pos += 2;
					break;
				}
				if(check(article, pos, "\n")){//special case where the @@ is not used to comment something out, it's just there
					buffer += "@@";
					pos = escapeStart;
					break;
				}
			}
			continue;
		}
		
		if(check(article, pos, "[!--")){
			emptyBuffer(buffer, output);
			pos += 4;
			std::size_t escapeStart = pos;
			for(; pos < article.size(); pos++){
				if(check(article, pos, "--]")){
					break;
				}
			}
			pos += 3;
			continue;
		}
		
		if(check(article, pos, "[[code]]")){
			emptyBuffer(buffer, output);
			pos += 8;
			std::size_t codeStart = pos;
			for(; pos < article.size(); pos++){
				if(check(article, pos, "[[/code]]")){
					break;
				}
			}
			output.push_back(Token(Token::Type::SectionComplete, "code", "", article.substr(codeStart, pos - codeStart)));
			pos += 9;
			continue;
		}
		
		if(check(article, pos, "[[html]]")){
			emptyBuffer(buffer, output);
			pos += 8;
			std::size_t codeStart = pos;
			for(; pos < article.size(); pos++){
				if(check(article, pos, "[[/html]]")){
					break;
				}
			}
			output.push_back(Token(Token::Type::SectionComplete, "html", "", article.substr(codeStart, pos - codeStart)));
			pos += 9;
			continue;
		}
		
		if(check(article, pos, "[[/")){
			emptyBuffer(buffer, output);
			std::size_t endPos = article.find("]]", pos);
			std::string contents = article.substr(pos + 3, endPos - pos - 3);
			std::transform(contents.begin(), contents.end(), contents.begin(), ::tolower);
			pos = endPos + 2;
			if(contents == "size"){
				output.push_back(Token(Token::Type::FormatMarker, "size", "", ""));
			}
			else{
				output.push_back(Token(Token::Type::SectionEnd, contents, "", ""));
			}
			continue;
		}
		
		if(check(article, pos, "[[[") || check(article, pos, "[http://") || check(article, pos, "[*http://")){
			std::string divider;
			std::string endTag;
			std::size_t beginOffset;
			if(check(article, pos, "[[[")){
				divider = "|";
				endTag = "]]]";
				pos += 3;
			}
			else{
				divider = " ";
				endTag = "]";
				pos++;
			}
			
			if(check(article, pos, "*")){
				pos++;
			}
			
			emptyBuffer(buffer, output);
			std::size_t endTagPos = article.find(endTag, pos);
			std::string fullContents = article.substr(pos, endTagPos - pos);
			std::size_t endLinkPos = fullContents.find(divider);
			std::string link;
			std::string shown;
			if(endLinkPos == std::string::npos){
				link = fullContents;
				shown = fullContents;
			}
			else{
				link = fullContents.substr(0, endLinkPos);
				shown = fullContents.substr(endLinkPos + divider.size(), fullContents.size() - endLinkPos - divider.size());
			}
			trimString(link);
			trimString(shown);
			replaceAll(link, " ", "-");
			std::transform(link.begin(), link.end(), link.begin(), ::tolower);
			pos = endTagPos + endTag.size();
			output.push_back(Token(Token::Type::SectionComplete, "hyperLink", link, shown));
			continue;
		}
		
		if(check(article, pos, "[[") && !isdigit(article[pos + 2])){
			emptyBuffer(buffer, output);
			
			std::size_t endTagPos = pos + 1;
			{
				int layer = 0;
				while(endTagPos < article.size()){
					endTagPos++;
					if(check(article, endTagPos, "]")){
						layer--;
						if(layer < 0){
							break;
						}
					}
					else if(check(article, endTagPos, "[")){
						layer++;
						if(layer < 0){
							break;
						}
					}
				}
			}
			std::string fullContents = article.substr(pos + 2, endTagPos - pos - 2);
			std::size_t endTypePos = fullContents.find(" ");
			std::string type;
			if(endTypePos != std::string::npos){
				type = fullContents.substr(0, endTypePos);
			}
			else{
				type = fullContents;
			}
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
			std::size_t startData = endTypePos + 1;
			if(type == "module" && endTypePos != std::string::npos){
				std::string name;
				std::size_t endNamePos = fullContents.find(" ", endTypePos + 1);
				name = fullContents.substr(endTypePos + 1, endNamePos - endTypePos - 1);
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);
				startData = endNamePos + 1;
				if(name == "rate"){
					output.push_back(Token(Token::Type::SectionComplete, type, "rate", ""));
					pos = endTagPos + 2;
					continue;
				}
				else if(name == "css"){
					pos = endTagPos + 2;
					std::size_t cssStart = pos;
					for(; pos < article.size(); pos++){
						if(check(article, pos, "[[/module]]")){
							break;
						}
					}
					std::string cssData = article.substr(cssStart, pos - cssStart);
					output.push_back(Token(Token::Type::SectionComplete, type, "css", cssData));
					pos += 11;
					continue;
				}
			}
			else if((type == "user" || type == "*user") && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				output.push_back(Token(Token::Type::SectionComplete, "user", "", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "footnoteblock"){
				output.push_back(Token(Token::Type::SectionComplete, "footnoteblock", "", ""));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "include"  && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				trimString(data);
				if(data.find("image-block") != std::string::npos){
					std::string caption;
					getData(data, "caption=", "|", 0, caption);
					output.push_back(Token(Token::Type::SectionStart, "imageBlock", "", data));
					{
						std::vector<Token> tokenizedCaption = tokenizeArticle(caption);
						output.insert(output.end(), tokenizedCaption.begin(), tokenizedCaption.end());
					}
					output.push_back(Token(Token::Type::SectionEnd, "imageBlock", "", ""));
					pos = endTagPos + 2;
					continue;
				}
				else if(data.find("sp-image") != std::string::npos){
					output.push_back(Token(Token::Type::SectionComplete, "image", "collapsible", data));
					pos = endTagPos + 2;
					continue;
				}
				else{
					output.push_back(Token(Token::Type::SectionComplete, "include", "", data));
					pos = endTagPos + 2;
					continue;
				}
			}
			else if(type == "toc" && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				output.push_back(Token(Token::Type::SectionComplete, "toc", "", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "size" && endTypePos != std::string::npos){
				//size is actually a type of FormatMarker
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				output.push_back(Token(Token::Type::FormatMarker, "size", "", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "image" && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				trimString(data);
				output.push_back(Token(Token::Type::SectionComplete, "image", "", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "<image" && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				trimString(data);
				output.push_back(Token(Token::Type::SectionComplete, "image", "<", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == "=image" && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				trimString(data);
				output.push_back(Token(Token::Type::SectionComplete, "image", "=", data));
				pos = endTagPos + 2;
				continue;
			}
			else if(type == ">image" && endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				trimString(data);
				output.push_back(Token(Token::Type::SectionComplete, "image", ">", data));
				pos = endTagPos + 2;
				continue;
			}
			if(endTypePos != std::string::npos){
				std::string data = fullContents.substr(startData, fullContents.size() - startData);
				output.push_back(Token(Token::Type::SectionStart, type, "", data));
				pos = endTagPos + 2;
				continue;
			}
			output.push_back(Token(Token::Type::SectionStart, type, "", ""));
			pos = endTagPos + 2;
			continue;
		}
		
		{
			if(isNewLine && article[pos] == ' ' || article[pos] == '\t'){//if the whitespace is at the beginning on a line
				overrideNewLine = true;//just ignore it and continue as if it was a newline
				pos += 1;
				continue;
			}
			else if(article[pos] == '\0'){
				if(isNewLine){
					overrideNewLine = true;
				}
				pos += 1;
				continue;
			}
			if(article[pos] == '\n'){
				std::cout << buffer << "\n";
				throw std::runtime_error("Attempted to add newline to buffer");
			}
			buffer += article[pos];
			pos += 1;
			continue;
		}
	}
	
	emptyBuffer(buffer, output);
	
	return output;
}

void Parser::printTokenizedArticle(std::vector<Token>& article){
	for(auto i = article.begin(); i != article.end(); i++){
		printToken(*i);
	}
}

void Parser::printToken(Token token){
	switch(token.type){
	case Token::Type::SectionStart:
		std::cout << "SectionStart: ";
		break;
	case Token::Type::SectionEnd:
		std::cout << "SectionEnd: ";
		break;
	case Token::Type::SectionComplete:
		std::cout << "SectionComplete: ";
		break;
	case Token::Type::PrefixFormat:
		std::cout << "PrefixFormat: ";
		break;
	case Token::Type::FormatMarker:
		std::cout << "FormatMarker: ";
		break;
	case Token::Type::NewLine:
		std::cout << "NewLine: ";
		break;
	case Token::Type::PlainText:
		std::cout << "PlainText: ";
		break;
	default:
		throw std::runtime_error("Attempted To Print Invalid Token");
	}
	
	std::cout << "\"" << token.sectionType << "\" \"" << token.sectionName << "\" \"" << token.data << "\"\n";
}

namespace{
	bool canInsertDivider(Statement::Type type){
		switch(type){
		case Statement::Type::Article:
		case Statement::Type::TabSection:
		case Statement::Type::Collapsible:
		case Statement::Type::Quote:
		case Statement::Type::Div:
		case Statement::Type::Footnote:
		case Statement::Type::CenterAlign:
		case Statement::Type::JustifyAlign:
		case Statement::Type::RightAlign:
		case Statement::Type::LeftAlign:
		case Statement::Type::TableElement://because they are allowed to have aligns in them
		case Statement::Type::ImageBlock://because of course somebody will want to do this
			return true;
			break;
		case Statement::Type::Header:
		case Statement::Type::Paragraph:
		case Statement::Type::TabView:
		case Statement::Type::OrderedList:
		case Statement::Type::BulletList:
		case Statement::Type::Table:
		case Statement::Type::TableRow:
		case Statement::Type::ListElement:
		case Statement::Type::Span:
		case Statement::Type::Color:
		case Statement::Type::Size:
		case Statement::Type::Italic:
		case Statement::Type::Bold:
		case Statement::Type::Underline:
		case Statement::Type::Strikethrough:
		case Statement::Type::Monospaced:
		case Statement::Type::Superscript:
		case Statement::Type::Subscript:
			return false;
			break;
		default:
			throw std::runtime_error("Attempted to call 'canInsertDivider' in a way that doesn't make sense");
		}
	}
	
	bool canInsertText(Statement::Type type){
		switch(type){
		case Statement::Type::Article:
		case Statement::Type::TabSection:
		case Statement::Type::Collapsible:
		case Statement::Type::Quote:
		case Statement::Type::Div:
		case Statement::Type::Footnote:
		case Statement::Type::CenterAlign:
		case Statement::Type::JustifyAlign:
		case Statement::Type::RightAlign:
		case Statement::Type::LeftAlign:
		case Statement::Type::TabView:
		case Statement::Type::OrderedList:
		case Statement::Type::BulletList:
		case Statement::Type::Table:
		case Statement::Type::TableRow:
			return false;
			break;
		case Statement::Type::Header:
		case Statement::Type::Paragraph:
		case Statement::Type::ImageBlock:
		case Statement::Type::ListElement:
		case Statement::Type::TableElement:
		case Statement::Type::Span:
		case Statement::Type::Color:
		case Statement::Type::Size:
		case Statement::Type::Italic:
		case Statement::Type::Bold:
		case Statement::Type::Underline:
		case Statement::Type::Strikethrough:
		case Statement::Type::Monospaced:
		case Statement::Type::Superscript:
		case Statement::Type::Subscript:
			return true;
			break;
		default:
			throw std::runtime_error("Attempted to call 'canInsertText' in a way that doesn't make sense, id = " + std::to_string(static_cast<int>(type)));
		}
	}
	
	bool shouldCarryOver(Statement::Type type){
		switch(type){
		case Statement::Type::Article:
		case Statement::Type::TabSection:
		case Statement::Type::Collapsible:
		case Statement::Type::Quote:
		case Statement::Type::Div:
		case Statement::Type::Footnote:
		case Statement::Type::TabView:
		case Statement::Type::OrderedList:
		case Statement::Type::BulletList:
		case Statement::Type::Table:
		case Statement::Type::TableRow:
		case Statement::Type::Header:
		case Statement::Type::Paragraph:
		case Statement::Type::ImageBlock:
		case Statement::Type::ListElement:
		case Statement::Type::TableElement:
		case Statement::Type::CenterAlign:
		case Statement::Type::JustifyAlign:
		case Statement::Type::RightAlign:
		case Statement::Type::LeftAlign:
			return false;
			break;
		case Statement::Type::Span:
		case Statement::Type::Color:
		case Statement::Type::Size:
		case Statement::Type::Italic:
		case Statement::Type::Bold:
		case Statement::Type::Underline:
		case Statement::Type::Strikethrough:
		case Statement::Type::Monospaced:
		case Statement::Type::Superscript:
		case Statement::Type::Subscript:
			return true;
			break;
		default:
			throw std::runtime_error("Attempted to call 'canInsertText' in a way that doesn't make sense, id = " + std::to_string(static_cast<int>(type)));
		}
	}
	
	void popStack(std::vector<Statement>& statementStack, Statement*& top){
		if(statementStack.size() <= 1){
			throw std::runtime_error("Attempted to pop the stack when it has 1 or less elements left");
		}
		
		Statement t = *top;
		statementStack.pop_back();
		top = &statementStack.back();
		top->statements.push_back(t);
		//std::cout << "pop";
	}
	
	void carryOverCarryOverStatements(std::vector<Statement>& statementStack, Statement*& top, std::stack<Statement>& carryOverStatements){
		while(shouldCarryOver(top->type)){
			Statement c = *top;
			c.statements.clear();
			carryOverStatements.push(c);
			popStack(statementStack, top);
		}
	}
	
	void makeInsertDivider(std::vector<Statement>& statementStack, Statement*& top, std::stack<Statement>& carryOverStatements){
		carryOverCarryOverStatements(statementStack, top, carryOverStatements);
		while(!canInsertDivider(top->type)){
			popStack(statementStack, top);
		}
	}
	
	void makeInsertText(std::vector<Statement>& statementStack, Statement*& top, std::stack<Statement>& carryOverStatements){
		if(!canInsertText(top->type)){
			makeInsertDivider(statementStack, top, carryOverStatements);
			Statement para;
			para.type = Statement::Type::Paragraph;
			statementStack.push_back(para);
		}
		while(carryOverStatements.size() > 0){
			statementStack.push_back(carryOverStatements.top());
			carryOverStatements.pop();
		}
		top = &statementStack.back();
	}
}

Statement Parser::statementizeArticle(std::vector<Token>& tokenizeArticle, std::string articleTitle){
	
	//because QuoteBlocks are handled on the token before the actual quoteblock token, articles must begin with NewLines
	tokenizeArticle.insert(tokenizeArticle.begin(), Token(Token::Type::NewLine, "", "", ""));
	
	std::vector<Statement> statementStack;
	{
		Statement state;
		state.type = Statement::Type::Article;
		state.extraDataA = articleTitle;
		statementStack.push_back(state);
	}
	
	int newLines = 2;
	Statement* top = &statementStack.back();
	
	std::stack<Statement> carryOverStatements;
	
	for(auto i = tokenizeArticle.begin(); i != tokenizeArticle.end(); i++){
		top = &statementStack.back();
		
		#ifdef DEBUG
		std::cout << "\n\n\n\n============\nCurrentNewLines:" << newLines << ", StackSize:" << statementStack.size() << "\nCurrent Token:";
		printToken(*i);
		std::cout << "\nCurrent Top:";
		top->print();
		#endif //DEBUG
		
		if(i->type == Token::Type::NewLine){
			newLines++;
			
			{//handle enter/leaving/nesting quote boxes
				int targetNesting;
				auto y = i;
				y++;
				if(y == tokenizeArticle.end()){
					continue;
				}
				if(y->type == Token::Type::PrefixFormat && y->sectionType == "quoteBlock"){
					targetNesting = y->numData;
				}
				else{
					targetNesting = 0;
				}
				
				int quotesNested = 0;
				for(auto y = statementStack.rbegin(); y != statementStack.rend(); y++){
					if(y->type == Statement::Type::Quote){
						quotesNested++;
					}
					if(y->type == Statement::Type::Footnote){
						break;
					}
				}
				
				if(targetNesting < quotesNested){
					carryOverCarryOverStatements(statementStack, top, carryOverStatements);
					while(targetNesting < quotesNested){
						while(top->type != Statement::Type::Quote){
							popStack(statementStack, top);//pop everything in the quote
						}
						//pop the quote statement
						popStack(statementStack, top);
						quotesNested--;
					}
					if(targetNesting > 0){
						i++;//skip the PrefixFormat that is going to appear after the newline, because we just handled it
					}
				}
				else if(targetNesting > quotesNested){
					carryOverCarryOverStatements(statementStack, top, carryOverStatements);
					while(targetNesting > quotesNested){
						makeInsertDivider(statementStack, top, carryOverStatements);
						Statement q;//add the new quote statement onto the stack
						q.type = Statement::Type::Quote;
						statementStack.push_back(q);
						quotesNested++;
						top = &statementStack.back();
					}
					i++;
				}
				else if(targetNesting > 0){
					i++;//if we're in the targetNesting, everything is fine and all we have to do is skip the QuoteBlock PrefixFormat
				}
			}
			continue;
		}
		else{
			if(newLines >= 1 && !(i->type == Token::Type::SectionEnd)){//"carry over" anything that can apply to more than one paragraph
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
			}
			
			if(newLines == 1){
				if((top->type == Statement::Type::Paragraph || top->type == Statement::Type::TableElement) && ((i->type == Token::Type::SectionComplete && i->sectionType == "hyperLink") || i->type == Token::Type::PlainText || i->type == Token::Type::FormatMarker)){
						//only add a LineBreak if the token being added can exist inside of the paragraph the LineBreak is being added to
					Statement newLine;
					newLine.type = Statement::Type::LineBreak;
					top->statements.push_back(newLine);
				}
				else if(top->type == Statement::Type::ListElement){
					popStack(statementStack, top);
				}
				else if(top->type == Statement::Type::Header){
					popStack(statementStack, top);
				}
			}
			else if(newLines >= 2){
				if(top->type == Statement::Type::Paragraph){
					popStack(statementStack, top);
				}
				else if(top->type == Statement::Type::TableElement){
					Statement newLine;
					newLine.type = Statement::Type::LineBreak;
					top->statements.push_back(newLine);
					top->statements.push_back(newLine);
				}
				else if(top->type == Statement::Type::ListElement){
					popStack(statementStack, top);
					popStack(statementStack, top);
				}
				else if(top->type == Statement::Type::Header){
					popStack(statementStack, top);
				}
			}
		}
		
		if(i->type == Token::Type::PlainText){
			if(top->type != Statement::Type::TabView){//if it's in a tabview root, just assume it shouldn't be there and don't put it in the tree
				makeInsertText(statementStack, top, carryOverStatements);
				Statement t;
				t.type = Statement::Type::Text;
				t.extraDataA = i->data;
				t.extraDataB = i->sectionType;//pass over any special tags that could be in the text, 99% of the time this is empty
				top->statements.push_back(t);
			}
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "listBullet"){
			if(top->type == Statement::Type::BulletList){
				Statement l;
				l.type = Statement::Type::ListElement;
				statementStack.push_back(l);
			}
			else{
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement b;
				b.type = Statement::Type::BulletList;
				statementStack.push_back(b);
				Statement l;
				l.type = Statement::Type::ListElement;
				statementStack.push_back(l);
			}
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "listNumber"){
			if(top->type == Statement::Type::OrderedList){
				Statement l;
				l.type = Statement::Type::ListElement;
				statementStack.push_back(l);
			}
			else{
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement b;
				b.type = Statement::Type::OrderedList;
				statementStack.push_back(b);
				Statement l;
				l.type = Statement::Type::ListElement;
				statementStack.push_back(l);
			}
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "tableMarker"){
			carryOverCarryOverStatements(statementStack, top, carryOverStatements);
			if(top->type == Statement::Type::Table && newLines >= 2){
				popStack(statementStack, top);
			}
			
			if(top->type == Statement::Type::Table){
				Statement r;
				r.type = Statement::Type::TableRow;
				statementStack.push_back(r);
				Statement e;
				e.type = Statement::Type::TableElement;
				e.extraDataA = i->sectionName;
				e.extraDataB = i->data;
				statementStack.push_back(e);
			}
			else if(top->type == Statement::Type::TableRow){
				throw std::runtime_error("Table formatting error");
			}
			else if(top->type == Statement::Type::TableElement){
				popStack(statementStack, top);//pop the current element
				Statement e;
				e.type = Statement::Type::TableElement;
				e.extraDataA = i->sectionName;
				e.extraDataB = i->data;
				statementStack.push_back(e);
			}
			else{
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement t;
				t.type = Statement::Type::Table;
				statementStack.push_back(t);
				Statement r;
				r.type = Statement::Type::TableRow;
				statementStack.push_back(r);
				Statement e;
				e.type = Statement::Type::TableElement;
				e.extraDataA = i->sectionName;
				e.extraDataB = i->data;
				statementStack.push_back(e);
			}
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "tableRowEndMarker"){
			carryOverCarryOverStatements(statementStack, top, carryOverStatements);
			if(top->type == Statement::Type::TableElement){
				popStack(statementStack, top);//pop the element
				popStack(statementStack, top);//pop the row
			}
			else if(top->type == Statement::Type::Table){//wikidot allows this for some strange reason
				Statement r;
				r.type = Statement::Type::TableRow;
				statementStack.push_back(r);
				Statement e;
				e.type = Statement::Type::TableElement;
				e.extraDataA = i->sectionName;
				e.extraDataB = i->data;
				statementStack.push_back(e);
				top = &statementStack.back();
				popStack(statementStack, top);//pop the element
				popStack(statementStack, top);//pop the row
			}
			else{
				throw std::runtime_error("Table formatting error");
			}
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "header"){
			makeInsertDivider(statementStack, top, carryOverStatements);
			Statement h;
			h.type = Statement::Type::Header;
			h.extraDataA = i->data;
			statementStack.push_back(h);
		}
		else if(i->type == Token::Type::PrefixFormat && i->sectionType == "quoteBlock"){
			throw std::runtime_error("Error on \"Token::Type::PrefixFormat 'QuoteBlock'\" Logic, Cannot Continue");
		}
		else if(i->type == Token::Type::FormatMarker){
			makeInsertText(statementStack, top, carryOverStatements);
			
			Statement f;
			
			if(i->sectionType == "italic"){
				f.type = Statement::Type::Italic;
			}
			else if(i->sectionType == "bold"){
				f.type = Statement::Type::Bold;
			}
			else if(i->sectionType == "underline"){
				f.type = Statement::Type::Underline;
			}
			else if(i->sectionType == "strikethrough"){
				f.type = Statement::Type::Strikethrough;
			}
			else if(i->sectionType == "monospaced"){
				f.type = Statement::Type::Monospaced;
			}
			else if(i->sectionType == "superscript"){
				f.type = Statement::Type::Superscript;
			}
			else if(i->sectionType == "subscript"){
				f.type = Statement::Type::Subscript;
			}
			else if(i->sectionType == "size"){
				f.type = Statement::Type::Size;
				f.extraDataA = i->data;
			}
			else if(i->sectionType == "color"){
				f.type = Statement::Type::Color;
				f.extraDataA = i->data;
			}
			else{
				throw std::runtime_error("Invalid format marker '" + i->sectionType + "'");
			}
			
			if(f.type != Statement::Type::Size && f.type != Statement::Type::Color){
				if(top->type == f.type){
					popStack(statementStack, top);
				}
				else{
					statementStack.push_back(f);
				}
			}
			else{
				if(top->type == f.type && f.extraDataA == ""){
					popStack(statementStack, top);
				}
				else if(f.extraDataA == ""){
					throw std::runtime_error("Attempted to add a size or color modifier with no size or color data");
				}
				else{
					statementStack.push_back(f);
				}
			}
		}
		else if(i->type == Token::Type::SectionComplete){
			if(i->sectionType == "module"){
				if(i->sectionName == "css"){
					makeInsertDivider(statementStack, top, carryOverStatements);
					Statement h;
					h.type = Statement::Type::HtmlBlock;
					h.extraDataA = "<style>" + i->data + "</style>";
					top->statements.push_back(h);
				}
				else{
					makeInsertText(statementStack, top, carryOverStatements);
					Statement t;
					t.type = Statement::Type::Error;
					t.extraDataA = "Unsupported \"Token::Type::SectionComplete\"";
					top->statements.push_back(t);
				}
			}
			else if(i->sectionType == "pageDivider"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement p;
				p.type = Statement::Type::PageDivider;
				top->statements.push_back(p);
			}
			else if(i->sectionType == "pageClearer"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement p;
				p.type = Statement::Type::PageClearer;
				top->statements.push_back(p);
			}
			else if(i->sectionType == "footnoteblock"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement f;
				f.type = Statement::Type::FootnoteBlock;
				top->statements.push_back(f);
			}
			else if(i->sectionType == "code"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::Code;
				c.extraDataA = i->data;
				top->statements.push_back(c);
			}
			else if(i->sectionType == "html"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::HtmlBlock;
				c.extraDataA = i->data;
				top->statements.push_back(c);
			}
			else if(i->sectionType == "hyperLink"){
				makeInsertText(statementStack, top, carryOverStatements);
				Statement l;
				l.type = Statement::Type::HyperLink;
				l.extraDataA = i->sectionName;
				l.extraDataB = i->data;
				top->statements.push_back(l);
			}
			else if(i->sectionType == "user"){
				makeInsertText(statementStack, top, carryOverStatements);
				Statement u;
				u.type = Statement::Type::User;
				u.extraDataA = i->data;
				top->statements.push_back(u);
			}
			else if(i->sectionType == "image"){
				if(i->sectionName == "collapsible"){
					makeInsertDivider(statementStack, top, carryOverStatements);
					{
						Statement h;
						h.type = Statement::Type::Header;
						h.extraDataA = "3";
						statementStack.push_back(h);
					}
					top = &statementStack.back();
					{
						Statement t;
						t.type = Statement::Type::Text;
						getData(i->data, "caption=", "|", 0, t.extraDataA);
						top->statements.push_back(t);
					}
					popStack(statementStack, top);
					{
						Statement c;
						c.type = Statement::Type::Collapsible;
						c.extraDataA = "+ Show image";
						c.extraDataB = "- Hide image";
						statementStack.push_back(c);
					}
					{
						Statement a;
						a.type = Statement::Type::CenterAlign;
						statementStack.push_back(a);
					}
					top = &statementStack.back();
					{
						Statement img;
						img.type = Statement::Type::Image;
						getData(i->data, "src=", "|", 0, img.extraDataA);
						{
							std::string temp;
							getData(i->data, "src=", " ", 0, temp);
							if(temp.size() < img.extraDataA.size()){
								img.extraDataA = temp;
								std::cout << "==========Using Image Link Capability Hack(on a collapsible image, which hasn't been tested)...\n";
							}
						}
						if(i->data.find("width=") != std::string::npos){
							getData(i->data, "width=", "|", 0, img.extraDataB);
							img.extraDataB = "style='width:" + img.extraDataB + "px;'";
						}
						else{
							img.extraDataB = "style='width:300px;'";
						}
						img.extraDataC = "block";
						top->statements.push_back(img);
					}
					popStack(statementStack, top);
					popStack(statementStack, top);
				}
				else{
					bool shouldPop = true;
					if(i->sectionName == ""){
						while(carryOverStatements.size() > 0){
							statementStack.push_back(carryOverStatements.top());
							carryOverStatements.pop();
						}
						top = &statementStack.back();
						shouldPop = false;
					}
					else if(i->sectionName == "<"){
						makeInsertDivider(statementStack, top, carryOverStatements);
						Statement a;
						a.type = Statement::Type::LeftAlign;
						statementStack.push_back(a);
					}
					else if(i->sectionName == "="){
						makeInsertDivider(statementStack, top, carryOverStatements);
						Statement a;
						a.type = Statement::Type::CenterAlign;
						statementStack.push_back(a);
					}
					else if(i->sectionName == ">"){
						makeInsertDivider(statementStack, top, carryOverStatements);
						Statement a;
						a.type = Statement::Type::RightAlign;
						statementStack.push_back(a);
					}
					top = &statementStack.back();
					{
						std::size_t endSrcPos = i->data.find(" ");
						Statement img;
						img.type = Statement::Type::Image;
						img.extraDataA = i->data.substr(0, endSrcPos);
						img.extraDataB = i->data.substr(endSrcPos + 1, i->data.size() - endSrcPos - 1);
						if(shouldPop){
							img.extraDataC = "block";
						}
						top->statements.push_back(img);
					}
					if(shouldPop){
						popStack(statementStack, top);
					}
				}
			}
			else{
				makeInsertText(statementStack, top, carryOverStatements);
				Statement t;
				t.type = Statement::Type::Error;
				t.extraDataA = "Unsupported \"Token::Type::SectionComplete\"";
				top->statements.push_back(t);
			}
		}
		else if(i->type == Token::Type::SectionStart){
			if(i->sectionType == "="){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::CenterAlign;
				statementStack.push_back(c);
			}
			else if(i->sectionType == "=="){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::JustifyAlign;
				statementStack.push_back(c);
			}
			else if(i->sectionType == ">"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::RightAlign;
				statementStack.push_back(c);
			}
			else if(i->sectionType == "<"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::LeftAlign;
				statementStack.push_back(c);
			}
			else if(i->sectionType == "span"){
				makeInsertText(statementStack, top, carryOverStatements);
				Statement s;
				s.type = Statement::Type::Span;
				s.extraDataA = i->data;
				statementStack.push_back(s);
			}
			else if(i->sectionType == "footnote"){
				makeInsertText(statementStack, top, carryOverStatements);
				Statement f;
				f.type = Statement::Type::Footnote;
				statementStack.push_back(f);
			}
			else if(i->sectionType == "div" || i->sectionType == "div_"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement s;
				s.type = Statement::Type::Div;
				s.extraDataA = i->data;
				statementStack.push_back(s);
			}
			else if(i->sectionType == "imageBlock"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement img;
				img.type = Statement::Type::ImageBlock;
				getData(i->data, "name=", "|", 0, img.extraDataA);
				{
					std::string temp;
					getData(i->data, "name=", " ", 0, temp);
					if(temp.size() < img.extraDataA.size()){
						img.extraDataA = temp;
						std::cout << "==========Using Image Link Capability Hack...\n";
					}
				}
				if(i->data.find("width=") != std::string::npos){
					getData(i->data, "width=", "|", 0, img.extraDataB);
				}
				statementStack.push_back(img);
			}
			else if(i->sectionType == "tabview"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::TabView;
				statementStack.push_back(c);
			}
			else if(i->sectionType == "tab"){
				if(top->type != Statement::Type::TabView){
					throw std::runtime_error("Attempted to add a [[tab]] while not inside of a [[tabview]]");
				}
				Statement c;
				c.type = Statement::Type::TabSection;
				c.extraDataA = i->data;
				statementStack.push_back(c);
			}
			else if(i->sectionType == "collapsible"){
				makeInsertDivider(statementStack, top, carryOverStatements);
				Statement c;
				c.type = Statement::Type::Collapsible;
				getData(i->data, "show=\"", "\"", 0, c.extraDataA);
				getData(i->data, "hide=\"", "\"", 0, c.extraDataB);
				statementStack.push_back(c);
			}
			else{
				makeInsertText(statementStack, top, carryOverStatements);
				Statement t;
				t.type = Statement::Type::Error;
				t.extraDataA = "Unsupported \"Token::Type::SectionStart\"";
				top->statements.push_back(t);
			}
		}
		else if(i->type == Token::Type::SectionEnd){
			if(i->sectionType == "="){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::CenterAlign){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "=="){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::JustifyAlign){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == ">"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::RightAlign){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "<"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::LeftAlign){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "span"){
				makeInsertText(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::Span){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "footnote"){
				while(top->type != Statement::Type::Footnote){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "div"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::Div){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "imageBlock"){
				while(top->type != Statement::Type::ImageBlock){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "tabview"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::TabView){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "tab"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::TabSection){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else if(i->sectionType == "collapsible"){
				carryOverCarryOverStatements(statementStack, top, carryOverStatements);
				while(top->type != Statement::Type::Collapsible){
					popStack(statementStack, top);
				}
				popStack(statementStack, top);
			}
			else{
				makeInsertText(statementStack, top, carryOverStatements);
				Statement t;
				t.type = Statement::Type::Error;
				t.extraDataA = "Unsupported \"Token::Type::SectionEnd\"";
				top->statements.push_back(t);
			}
		}
		else{
			makeInsertText(statementStack, top, carryOverStatements);
			Statement t;
			t.type = Statement::Type::Error;
			t.extraDataA = "Unsupported Token::Type::Unknown";
			//shrug, I have no idea what it could be that is missed, so just put a marker to whoever is reading the thing knows that *something* was missed and it needs to be added
			top->statements.push_back(t);
		}
		
		newLines = 0;
	}
	
	while(statementStack.size() > 1){
		popStack(statementStack, top);
	}
	
	return statementStack.back();
}

std::string Parser::convertToHtml(Statement& article, bool makePrinterFriendly, bool downloadImages, std::string imageUrlPath, std::string imageSavePath){
	ParserConvertData convertData;
	convertData.uniqueNumber = 0;
	convertData.printerFriendly = makePrinterFriendly;
	convertData.downloadImages = downloadImages;
	convertData.imageUrlPath = imageUrlPath;
	convertData.imageSavePath = imageSavePath;
	
	std::string output = "<!DOCTYPE html>\n<html lang='en-US'><head>"
	"<style>"
	//remove annoying margin after headers
	"h1{margin-top:1rem;margin-bottom:0.5rem;font-size:2rem;}"
	"h2{margin-bottom:0.5rem;font-size:1.3rem;}"
	"h3{margin-bottom:0.5rem;font-size:1.1rem;}"
	"h4{margin-bottom:0.5rem;font-size:1rem;}"
	"h5{margin-bottom:0.5rem;font-size:0.8rem;}"
	"h6{margin-bottom:0.5rem;font-size:0.7rem;}"
	
	"body{width:50rem;margin:auto;font-family:sans-serif;}"
	"p{line-height:1.25;}"
	"li{line-height:1.25;}"
	
	".PageClearer{display:block;clear:both;}"
	"blockquote{border-width:1px;border-style:dashed;border-color:black;padding-left:1rem;padding-right:1rem;}"
	"code{display:block;border-width:1px;border-style:dashed;border-color:gray;background-color:lightgray;}"
	".CenterAlign{text-align:center;}"
	".JustifyAlign{text-align:justify;}"
	".RightAlign{text-align:right;}"
	".LeftAlign{text-align:left;}"
	".Monospaced{font-family:'Terminus', monospace;font-size:inherit;}"
	".Underline{text-decoration:underline;}"
	".Strikethrough{text-decoration:line-through;}"
	//normal images
	".ImageBlockCaption>img{display:inline;}"
	"div>img{display:block;}"
	".ImageContainer{display:inline-block;width:auto;height:auto;}"
	".CenterAlign .ImageContainer{margin-left:auto;margin-right:auto;}"
	".LeftAlign .ImageContainer{margin-left:0px;margin-right:auto;}"
	".Right .ImageContainer{margin-left:auto;margin-right:0px;}"
	//image blocks
	".ImageBlock{text-align:center;display:block;width:300px;float:right;clear:right;margin:1rem 1rem 1rem 1rem;border-style:solid;border-color:black;border-width:1px;}"
	"blockquote>.ImageBlock{margin:1rem 3rem 1rem 1rem;}"
	".ImageBlock>img{display:block;width:100%;padding:0px;}"
	".ImageBlock>.ImageBlockCaption{display:block;box-sizing:border-box;width:100%;border-style:solid;border-color:black;border-width:0px;border-top-width:1px;padding:0.2rem;font-size:0.9rem;}"
	//tables
	"table{border-collapse:collapse;margin:0.5rem;empty-cells:show;}"
	"tr{border-collapse:collapse;}"
	"td{border-collapse:collapse;border-style:solid;border-width:1px;border-color:black;padding:0.5rem;}"
	"th{border-collapse:collapse;border-style:solid;border-width:1px;border-color:black;padding:0.5rem;}"
	".TableElementLeftAlign{text-align:left;}"
	".TableElementRightAlign{text-align:right;}"
	".TableElementCenterAlign{text-align:center;}"
	//for collapsibles
	".CollapsibleContainer{}"
	".CollapsibleContent{display:none;}"
	".CollapsibleHiddenText{cursor:pointer;}"
	".CollapsibleHiddenText:hover{text-decoration:underline;}"
	".CollapsibleShownText{cursor:pointer;display:none;}"
	".CollapsibleShownText:hover{text-decoration:underline;}"
	".CollapsibleContainer>input[type=checkbox]{display:none;}"
	//when the collapsible is opened
	"input[type=checkbox]:checked~.CollapsibleContent{display:block;}"
	"input[type=checkbox]:checked~.CollapsibleHiddenText{display:none;}"
	"input[type=checkbox]:checked~.CollapsibleShownText{display:block;}"
	//for tabviewsTabView,TabViewButtons,TabViewButton,TabSection,TabSectionTitle
	".TabView{}"
	".TabViewButtons{}"
	".TabViewButton{cursor:pointer;border-style:solid;border-color:black;border-width:1px;border-bottom-width:0px;display:inline-block;padding:2px;margin-right:0.5rem;}"
	".TabViewButton:hover{text-decoration:underline;}"
	".TabView>input[type=radio]{display:none;}"
	".TabSection{padding:0.5rem;padding-top:0px;display:none;border-style:solid;border-color:black;border-width:1px;overflow:hidden;}"
	".TabSectionTitle{display:inline-block;padding:2px;padding-left:4rem;font-family:monospace;}"
	//for showing the selected tabview
	"input[type=radio]:checked+.TabSection{display:block;}"
	
	//my version of some of the default stuff the wiki adds, so custom divs and stuff can still use them
	".scp-image-block{border:solid 1px black;width:300px;}"
	".scp-image-block.block-left{float:left;clear:left;margin:0 2rem 1rem 0;}"
	".scp-image-block.block-right{float:right;clear:right;margin:0 2rem 1rem 0;}"
	".scp-image-block.block-center{margin-left:auto;margin-right:auto;}"
	".scp-image-block.scp-image-caption{display:block;box-sizing:border-box;width:300px;border-style:solid;border-color:black;border-width:0px;border-top-width:1px;padding:0.2rem;font-size:0.9rem;}"
	;
	
	if(convertData.printerFriendly){
		output +=
		".footer-wikiwalk-nav{display:none;}"
		"a{text-decoration:inherit;color:inherit;}"
		".CollapsibleContent{display:block;}"
		".CollapsibleHiddenText{display:none;}"
		".CollapsibleShownText{display:block;}";
	}
	else{
		output +=
		"html{font-size:80%;}"
		"body{width:60rem;}";
	}
	
	output += 
	"</style>"
	"<meta charset='UTF-8'></head><body>";
	article.toRawHtml(output, convertData);
	output += "<body></html>";
	return output;
}






















