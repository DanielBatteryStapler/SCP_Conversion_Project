#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <iostream>
#include <functional>

namespace Parser{
	
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
	
	enum class DividerType{Unknown};
	
	struct Divider{
		using Type = DividerType;
		Type type = Type::Unknown;
		
		bool operator==(const Divider& tok)const;
	};
	
	enum class PrefixFormatType{Unknown};
	
	struct PrefixFormat{
		using Type = PrefixFormatType;
		Type type = Type::Unknown;
		
		bool operator==(const PrefixFormat& tok)const;
	};
	
	enum class NestingPrefixFormatType{Unknown};
	
	struct NestingPrefixFormat{
		using Type = NestingPrefixFormatType;
		Type type = Type::Unknown;
		unsigned int degree;
		
		bool operator==(const NestingPrefixFormat& tok)const;
	};
	
	struct InlineSectionStart : public SectionStart{
		
	};
	
	struct InlineSectionEnd : public SectionEnd{
		
	};
	
	enum class InlineFormatType{Unknown};
	
	struct InlineFormat{
		using Type = InlineFormatType;
		Type type;
		
		bool operator==(const InlineFormat& tok)const;
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
	
	enum class TokenType{Unknown = 0, SectionStart, SectionEnd, SectionComplete, Divider, PrefixFormat, NestingPrefixFormat, 
						InlineSectionStart, InlineSectionEnd, InlineFormat, LiteralText, PlainText, LineBreak, NewLine};
	
	struct Token{
		using Type = TokenType;
		std::variant<std::monostate, SectionStart, SectionEnd, SectionComplete, Divider, PrefixFormat, NestingPrefixFormat, 
						InlineSectionStart, InlineSectionEnd, InlineFormat, LiteralText, PlainText, LineBreak, NewLine> token;
		
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
