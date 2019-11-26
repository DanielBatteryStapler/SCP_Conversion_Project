#ifndef TREER_HPP
#define TREER_HPP

#include "Parser.hpp"

namespace Parser{
	
	struct StyleFormat{
		using Type = InlineFormat::Type;
		Type type;
		
		std::string color;//optional
		
		bool operator==(const StyleFormat& nod)const;
	};
	
	struct Paragraph{
		bool operator==(const Paragraph& nod)const;
	};
	
	struct List{
        using Type = ListPrefix::Type;
        Type type = Type::Unknown;
		bool operator==(const List& nod)const;
    };
    
    struct ListElement{
		bool operator==(const ListElement& nod)const;
    };
	
	struct QuoteBox{
		bool operator==(const QuoteBox& nod)const;
	};
	
	struct RootPage{
		bool operator==(const RootPage& nod)const;
	};
	
	enum class NodeType{Unknown = 0, RootPage, QuoteBox, List, ListElement, Paragraph, Heading, Divider, LineBreak, PlainText, LiteralText, HyperLink, StyleFormat};
	using NodeVariant = std::variant<std::monostate, RootPage, QuoteBox, List, ListElement, Paragraph, Heading, Divider, LineBreak, PlainText, LiteralText, HyperLink, StyleFormat>;
	
	struct Node{
		using Type = NodeType;
		using Variant = NodeVariant;
		Variant node;
		std::vector<Node> branches;
		
		Type getType()const;
		bool operator==(const Node& nod)const;
	};
	
	std::string toString(const Node& nod, int tab = 0);
	std::ostream& operator<<(std::ostream& out, const Node& nod);
	
	struct PageTree{
		Node pageRoot;
	};
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage);
	PageTree makeTreeFromPage(std::string page);
}

#endif // TREER_HPP
