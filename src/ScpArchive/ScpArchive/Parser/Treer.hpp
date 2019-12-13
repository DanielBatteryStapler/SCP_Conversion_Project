#ifndef TREER_HPP
#define TREER_HPP

#include "Parser.hpp"

namespace Parser{
	
	struct Size{
        std::string size;
        
        bool operator==(const Size& nod)const;
	};
	
	struct Span{
        std::map<std::string, std::string> parameters;
        
        bool operator==(const Span& nod)const;
	};
	
	struct Div{
        std::map<std::string, std::string> parameters;
        
        bool operator==(const Div& nod)const;
	};
	
	enum class AlignType{Unknown, Left, Right, Center, Justify};
	struct Align{
        using Type = AlignType;
        Type type = Type::Unknown;
        
        bool operator==(const Align& nod)const;
	};
	
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
	
	enum class NodeType{Unknown = 0, RootPage, QuoteBox, Div, Align, List, ListElement, Paragraph, Heading, Divider, LineBreak, PlainText, LiteralText, HyperLink, StyleFormat, Span, Size};
	using NodeVariant = std::variant<std::monostate, RootPage, QuoteBox, Div, Align, List, ListElement, Paragraph, Heading, Divider, LineBreak, PlainText, LiteralText, HyperLink, StyleFormat, Span, Size>;
	
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
	
	struct CSS{
		std::string data;
		
		bool operator==(const CSS& css)const;
	};
	
	struct PageTree{
		Node pageRoot;
		std::vector<CSS> cssData;
		
		bool operator==(const PageTree& page)const;
	};
	
	std::ostream& operator<<(std::ostream& out, const PageTree& page);
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage);
	PageTree makeTreeFromPage(std::string page);
	
	struct TreeContext{
        std::vector<Node> stack;
        std::vector<Node> styleCarry;
        int newlines;
        std::size_t tokenPos;
        TokenedPage tokenedPage;
        
        std::vector<CSS> cssData;
    };
    
    struct TreeRule{
        struct TreeRuleType{
            Token::Type mainType;
            SectionType sectionType = SectionType::Unknown;
            ModuleType moduleType = ModuleType::Unknown;
        };
        TreeRuleType type;
        std::function<void(TreeContext&, const Token&)> handleRule;
    };
    
    bool isTextType(SectionType type);
    bool isTextType(const Token& tok);
    bool isTextInsertable(Node::Type type);
    bool isDivInsertable(Node::Type type);
    bool isStyleCarryable(Node::Type type);
    
    bool isInside(TreeContext& context, Node::Type type);
    
    void pushStack(TreeContext& context, Node newNode);
    void popStack(TreeContext& context);
    void popStackWithCarry(TreeContext& context);
    void popSingle(TreeContext& context, std::function<bool(const Node&)> check);
    void popSingle(TreeContext& context, Node::Type type);
    
    void makeDivPushable(TreeContext& context);
    void makeTextAddable(TreeContext& context);
    void addAsDiv(TreeContext& context, Node newNode);
    void addAsText(TreeContext& context, Node newNode);
}

#endif // TREER_HPP
