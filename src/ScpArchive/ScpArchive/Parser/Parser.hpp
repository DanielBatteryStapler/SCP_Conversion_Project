#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <iostream>
#include <functional>

class Database;

namespace Parser{
	std::string& trimLeft(std::string& s);
	std::string& trimRight(std::string& s);
	std::string& trimString(std::string& s);
	std::string normalizePageName(std::string link);
	
	enum class SectionType{Unknown, Module, FootNote, FootNoteBlock, TableOfContents, AdvTable, AdvTableRow, AdvTableElement, 
        HTML, IFrame, Iftags, Include, Image, TabView, Tab, Collapsible, Span, Size, Anchor, Align, Div, Code, A};
	enum class ModuleType{Unknown, CSS, Rate};
	
	struct Section{
		SectionType type = SectionType::Unknown;
		std::string typeString;
		ModuleType moduleType = ModuleType::Unknown;
		std::string mainParameter;
		std::map<std::string, std::string> parameters;
		
		bool operator==(const Section& tok)const;
	};
	
	struct SectionStart : public Section{
        
	};
	
	struct SectionEnd{
		using Type = SectionType;
		Type type = Type::Unknown;
		std::string typeString;
		
		bool operator==(const SectionEnd& tok)const;
	};
	
	struct SectionComplete : public SectionStart{
		std::string contents;
		
		bool operator==(const SectionComplete& tok)const;
	};
	
    enum class DividerType{Unknown, Line, ClearBoth, ClearLeft, ClearRight, Seperator};
	
	struct Divider{
		using Type = DividerType;
		Type type = Type::Unknown;
		
		bool operator==(const Divider& tok)const;
	};
	
	struct Heading{
		unsigned int degree;
		bool hidden;
		
		unsigned int tocNumber;
		
		bool operator==(const Heading& tok)const;
	};
	
	struct QuoteBoxPrefix{
		unsigned int degree;
		
		bool operator==(const QuoteBoxPrefix& tok)const;
	};
	
	enum class ListPrefixType{Unknown, Bullet, Number};
	struct ListPrefix{
		using Type = ListPrefixType;
		Type type = Type::Unknown;
		unsigned int degree;
		
		bool operator==(const ListPrefix& tok)const;
	};
	
	enum class InlineFormatType{Unknown, Strike, Italics, Bold, Underline, Super, Sub, Monospace, Color};
	
	struct InlineFormat{
		using Type = InlineFormatType;
		Type type;
		bool begin;
		bool end;
		
		std::string color;//optional
		
		bool operator==(const InlineFormat& tok)const;
	};
	
	struct TableMarker{
        enum class Type{Start, Middle, End, StartEnd};
        Type type;
        enum class AlignmentType{Default, Header, Left, Right, Center};
        AlignmentType alignment;
        unsigned int span;
        
        bool operator==(const TableMarker& tok)const;
	};
	
	struct HyperLink{
		std::string shownText;
		std::string url;
		bool newWindow;
		
		bool operator==(const HyperLink& tok)const;
	};
	
	struct LiteralText{
        std::string text;
        
        bool operator==(const LiteralText& tok)const;
	};
	
	struct PlainText{
		std::string text;
		
		bool operator==(const PlainText& tok)const;
	};
	
	struct CenterText{
        bool operator==(const CenterText& tok)const;
	};
	
	struct LineBreak{
		bool operator==(const LineBreak& tok)const;
	};
	
	struct NewLine{
		bool operator==(const NewLine& tok)const;
	};
	
	enum class TokenType{Unknown = 0, Section, SectionStart, SectionEnd, SectionComplete, Divider, Heading, CenterText, QuoteBoxPrefix, ListPrefix, 
						TableMarker, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine};
	
	using TokenVariant = std::variant<std::monostate, Section, SectionStart, SectionEnd, SectionComplete, Divider, Heading, CenterText, QuoteBoxPrefix, ListPrefix, 
                                        TableMarker, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine>;
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
	
	std::string tokenVariantToString(const Token& token);
	std::string toString(const Token& token);
	std::ostream& operator<<(std::ostream& out, const Token& tok);
	
	struct TokenedPage{
		std::vector<Token> tokens;
		std::string originalPage;
	};
	
	struct PageInfo{
        std::string name;
        std::vector<std::string> tags;
        int rating;
	};
	
	struct ParserParameters{
	    PageInfo page;
        std::map<std::string, std::string> includeParameters;
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
};

#endif // PARSER_HPP
