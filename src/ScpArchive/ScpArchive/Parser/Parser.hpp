#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <iostream>
#include <functional>

namespace Parser{
	std::string& trimLeft(std::string& s);
	std::string& trimRight(std::string& s);
	std::string& trimString(std::string& s);
	std::string normalizePageName(std::string link);
	
	enum class SectionType{Unknown};
	enum class ModuleType{Unknown};
	
	struct SectionStart{
		using Type = SectionType;
		Type type = Type::Unknown;
		ModuleType moduleType = ModuleType::Unknown;
		std::map<std::string, std::string> parameters;
		
		bool operator==(const SectionStart& tok)const;
	};
	
	struct SectionEnd{
		using Type = SectionType;
		Type type = Type::Unknown;
		
		bool operator==(const SectionEnd& tok)const;
	};
	
	struct SectionComplete : public SectionStart{
		
	};
	
    enum class DividerType{Unknown, Line, Clear};
	
	struct Divider{
		using Type = DividerType;
		Type type = Type::Unknown;
		
		bool operator==(const Divider& tok)const;
	};
	
	struct Heading{
		unsigned int degree;
		bool hidden;
		
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
	
	struct InlineSectionStart : public SectionStart{
		
	};
	
	struct InlineSectionEnd : public SectionEnd{
		
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
	
	struct LineBreak{
		bool operator==(const LineBreak& tok)const;
	};
	
	struct NewLine{
		bool operator==(const NewLine& tok)const;
	};
	
	enum class TokenType{Unknown = 0, SectionStart, SectionEnd, SectionComplete, Divider, Heading, QuoteBoxPrefix, ListPrefix, 
						InlineSectionStart, InlineSectionEnd, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine};
	
	using TokenVariant = std::variant<std::monostate, SectionStart, SectionEnd, SectionComplete, Divider, Heading, QuoteBoxPrefix, ListPrefix, 
							InlineSectionStart, InlineSectionEnd, InlineFormat, HyperLink, LiteralText, PlainText, LineBreak, NewLine>;
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
	
	std::string toString(const Token& token);
	std::ostream& operator<<(std::ostream& out, const Token& tok);
	
	struct TokenedPage{
		std::vector<Token> tokens;
		std::string originalPage;
	};
	
	TokenedPage tokenizePage(std::string page);
	std::vector<std::string> getPageLinks(std::string page);
	
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
	
	struct TokenRule{
		std::string name;
		std::function<bool(const TokenRuleContext&)> tryRule;
		std::function<TokenRuleResult(const TokenRuleContext&)> doRule;
	};
};

#endif // PARSER_HPP
