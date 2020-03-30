#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <iostream>
#include <functional>
#include <nlohmann/json.hpp>

#include "../Config.hpp"

class Database;

namespace Parser{
	std::string& trimLeft(std::string& s);
	std::string& trimRight(std::string& s);
	std::string& trimString(std::string& s);
	std::string normalizePageName(std::string link);
	
	enum class SectionType{Unknown, Module, FootNote, FootNoteBlock, TableOfContents, AdvTable, AdvTableRow, AdvTableElement, 
        HTML, IFrame, Iftags, Include, Image, TabView, Tab, Collapsible, Span, Size, Anchor, Align, Div, Code, A,
        AdvList, AdvListElement, User, SCPConversionProjectInfoBox};
	enum class ModuleType{Unknown, CSS, Rate, ForumStart, ForumCategory, ForumThread, ListPages, Redirect};
	
	struct Section{
		SectionType type = SectionType::Unknown;
		std::string typeString;
		ModuleType moduleType = ModuleType::Unknown;
		std::string mainParameter;
		std::map<std::string, std::string> parameters;
	};
	
	struct SectionStart : public Section{
        
	};
	
	struct SectionEnd{
		using Type = SectionType;
		Type type = Type::Unknown;
		std::string typeString;
	};
	
	struct SectionComplete : public SectionStart{
		std::string contents;
	};
	
    enum class DividerType{Unknown, Line, ClearBoth, ClearLeft, ClearRight, Separator};
	
	struct Divider{
		using Type = DividerType;
		Type type = Type::Unknown;
	};
	
	struct Heading{
		unsigned int degree;
		bool hidden;
		
		std::optional<unsigned int> tocNumber;//only used when in a Node tree and not hidden
	};
	
	struct QuoteBoxPrefix{
		unsigned int degree;
	};
	
	enum class ListPrefixType{Unknown, Bullet, Number};
	struct ListPrefix{
		using Type = ListPrefixType;
		Type type = Type::Unknown;
		unsigned int degree;
	};
	
	enum class InlineFormatType{Unknown, Strike, Italics, Bold, Underline, Super, Sub, Monospace, Color};
	
	struct InlineFormat{
		using Type = InlineFormatType;
		Type type;
		bool begin;
		bool end;
		
		std::string color;//optional
	};
	
	struct TableMarker{
        enum class Type{Start, Middle, End, StartEnd};
        Type type;
        enum class AlignmentType{Default, Header, Left, Right, Center};
        AlignmentType alignment;
        unsigned int span;
	};
	
	struct HyperLink{
		std::string shownText;
		std::string url;
		bool newWindow;
	};
	
	struct LiteralText{
        std::string text;
	};
	
	struct PlainText{
		std::string text;
	};
	
	struct CenterText{
	};
	
	struct LineBreak{
	};
	
	struct NewLine{
	};
	
	const inline std::vector<std::string> TokenTypeNames =
		{"Unknown", "Section", "SectionStart", "SectionEnd", "SectionComplete", "Divider", "Heading", "CenterText", "QuoteBoxPrefix",
		"ListPrefix", "TableMarker", "InlineFormat", "HyperLink", "LiteralText", "PlainText", "LineBreak", "NewLine"};
	
	enum class TokenType
		{Unknown = 0, Section, SectionStart, SectionEnd, SectionComplete, Divider, Heading, CenterText, QuoteBoxPrefix,
		ListPrefix, TableMarker, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine};
	
	using TokenVariant = std::variant
		<std::monostate, Section, SectionStart, SectionEnd, SectionComplete, Divider, Heading, CenterText, QuoteBoxPrefix,
		ListPrefix, TableMarker, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine>;
	
	struct Token{
		using Type = TokenType;
		using Variant = TokenVariant;
		Variant token;
		
		Type getType()const;
		
		std::size_t sourceStart;
		std::size_t sourceEnd;
		std::string source;
		
		bool operator==(const Token& tok)const;
	};
	
	std::string getTokenTypeName(Token::Type type);
	nlohmann::json printTokenVariant(const Token& token);
	nlohmann::json printToken(const Token& token);
	std::ostream& operator<<(std::ostream& out, const Token& tok);
	
	struct TokenedPage{
		std::vector<Token> tokens;
		std::string originalPage;
	};
	
	struct PageInfo{
        std::string name;
        std::string title;
        std::optional<std::string> parent;
        std::optional<std::string> author;
        std::vector<std::string> tags;
        std::int64_t rating;
        std::int64_t votes;
		TimeStamp creationTimeStamp;
	};
	
	struct ParserParameters{
	    PageInfo page;
        std::map<std::string, std::string> includeParameters;
        std::map<std::string, std::string> urlParameters;
        Database* database = nullptr;
        int includeDepth = 0;
	};
	
	struct TokenRuleContext{
		std::string page;
		std::size_t pagePos;
		std::vector<Token> tokens;
		bool wasNewLine;
	};
	
	struct TokenRuleResult{
		std::size_t newPos;
		bool nowNewline = false;
		std::vector<Token> newTokens;
	};
	
	TokenedPage tokenizePage(std::string page, ParserParameters parameters = {});
	std::vector<std::string> getPageLinks(std::string page);
}

#endif // PARSER_HPP
