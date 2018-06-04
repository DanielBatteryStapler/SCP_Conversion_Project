#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stack>
#include "Helpers.h"

struct Token{
	enum class Type{SectionStart, SectionEnd, SectionComplete, PrefixFormat, FormatMarker, PlainText, NewLine};
	Type type;
	std::string sectionType;
	std::string sectionName;
	std::string data;
	int numData;
	
	Token() = default;
	Token(Type _type, std::string _sectionType, std::string _sectionName, std::string _data, int _numData = 0);
};

struct ParserConvertData;

class Statement{
public:
	Statement() = default;
	
	enum class Type{Article, TabSection, Collapsible, Quote, Div, Footnote, CenterAlign, JustifyAlign, RightAlign, LeftAlign,
							Header, FootnoteBlock, HtmlBlock, ImageBlock, Image, PageDivider, PageClearer, Paragraph, TabView, OrderedList, BulletList, Table, TableRow,
								ListElement, TableElement, Span, Color, Size, Italic, Bold, Underline, Strikethrough, Monospaced, Superscript, Subscript,
									Error, Text, LineBreak, HyperLink, Code, User};
	Type type;
	
	std::string extraDataA;//ImageUrl, section titles, linkURL, size
	std::string extraDataB;//ImageWidth, ImageStyle, shownLink
	std::string extraDataC;//ImageBlockOrNot
	std::vector<Statement> statements;
	
	void print(int tab = 0);
	void toRawHtml(std::string& output, ParserConvertData& convertData);
};

struct ParserConvertData{
	std::size_t uniqueNumber;
	bool printerFriendly;
	bool downloadImages;
	std::string imageUrlPath;
	std::string imageSavePath;
	std::vector<Statement> footnotes;
};

class Parser{
public:
	Parser() = delete;
	
	static std::vector<Token> tokenizeArticle(std::string& article);
	static void printTokenizedArticle(std::vector<Token>& article);
	static void printToken(Token token);
	static Statement statementizeArticle(std::vector<Token>& tokenizeArticle, std::string articleTitle);
	
	static std::string convertToHtml(Statement& article, bool makePrinterFriendly, bool downloadImages, std::string imageUrlPath, std::string imageSavePath);
};



