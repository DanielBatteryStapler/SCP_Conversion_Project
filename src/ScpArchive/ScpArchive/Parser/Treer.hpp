#ifndef TREER_HPP
#define TREER_HPP

#include "Parser.hpp"

namespace Parser{
	
	struct Size{
        std::string size;
	};
	
	struct Span{
        std::map<std::string, std::string> parameters;
	};
	
	struct Anchor{
		std::string name;
	};
	
	struct Div{
        std::map<std::string, std::string> parameters;
	};
	
	struct A{
		std::map<std::string, std::string> parameters;
	};
	
	enum class AlignType{Unknown, Left, Right, Center, Justify};
	struct Align{
        using Type = AlignType;
        Type type = Type::Unknown;
	};
	
	struct StyleFormat{
		using Type = InlineFormat::Type;
		Type type;
		
		std::string color;//optional
	};
	
	struct Paragraph{
	};
	
	struct List{
        using Type = ListPrefix::Type;
        Type type = Type::Unknown;
    };
    
    struct ListElement{
    };
	
	struct AdvList{
		using Type = ListPrefix::Type;
        Type type = Type::Unknown;
        
        std::map<std::string, std::string> parameters;
	};
	
	struct AdvListElement{
        std::map<std::string, std::string> parameters;
	};
	
	struct QuoteBox{
	};
	
	struct Collapsible{
        std::string closedText;
        std::string openedText;
        bool defaultShow;
	};
	
	struct Image{
        std::string source;
        bool newWindow;
        std::string link;
        std::string alt;
        std::string title;
        std::string width;
        std::string height;
        std::string style;
        std::string cssClass;
        enum class AlignmentType{Default, Center, Left, Right, FloatLeft, FloatRight};
        AlignmentType alignment = AlignmentType::Default;
	};
    
    struct Code{
        std::string contents;
        std::string type;
        
		bool operator==(const Code& code)const;//needs this so std::vector<Code>'s can be compared
    };
    
    struct IFrame{
        std::string source;
        std::map<std::string, std::string> parameters;
    };
    
    struct HTML{
        std::string contents;
    };
    
    struct TabView{
    };
    
    struct Tab{
        std::string title;
    };
    
    struct TableOfContents{
        enum class AlignmentType{Default, FloatLeft, FloatRight};
        AlignmentType alignment = AlignmentType::Default;
    };
    
    struct FootNote{
        unsigned int number;
    };
    
    struct FootNoteBlock{
        std::string title;
    };
    
    struct Table{
    };
    
    struct TableRow{
    };
    
    struct TableElement{
        using AlignmentType = TableMarker::AlignmentType;
        AlignmentType alignment;
        unsigned int span;
    };
    
    struct AdvTable{
        std::map<std::string, std::string> parameters;
    };
    
    struct AdvTableRow{
        std::map<std::string, std::string> parameters;
    };
    
    struct AdvTableElement{
        bool isHeader;
        std::map<std::string, std::string> parameters;
    };
    
    struct Rate{
        int rating;
    };
    
	struct RootPage{
	};
	
	const inline std::vector<std::string> NodeTypeNames =
		{"Unknown", "RootPage", "Table", "TableRow", "TableElement", "AdvTable", "AdvTableRow", "AdvTableElement",
		"FootNote", "FootNoteBlock", "TableOfContents", "TabView", "Tab", "Collapsible", "Rate", "Image", "HTML", "IFrame", "Code",
		"QuoteBox", "Div", "Align", "List", "ListElement", "AdvList", "AdvListElement", "Paragraph", "CenterText", "Heading", "Divider",
		"LineBreak", "PlainText", "LiteralText", "HyperLink", "StyleFormat", "Span", "Anchor", "Size", "A"};
		
	enum class NodeType
		{Unknown = 0, RootPage, Table, TableRow, TableElement, AdvTable, AdvTableRow, AdvTableElement,
        FootNote, FootNoteBlock, TableOfContents, TabView, Tab, Collapsible, Rate, Image, HTML, IFrame, Code,
        QuoteBox, Div, Align, List, ListElement, AdvList, AdvListElement, Paragraph, CenterText, Heading, Divider,
        LineBreak, PlainText, LiteralText, HyperLink, StyleFormat, Span, Anchor, Size, A};
        
	using NodeVariant = std::variant
		<std::monostate, RootPage, Table, TableRow, TableElement, AdvTable, AdvTableRow, AdvTableElement,
        FootNote, FootNoteBlock, TableOfContents, TabView, Tab, Collapsible, Rate, Image, HTML, IFrame, Code,
        QuoteBox, Div, Align, List, ListElement, AdvList, AdvListElement, Paragraph, CenterText, Heading, Divider,
        LineBreak, PlainText, LiteralText, HyperLink, StyleFormat, Span, Anchor, Size, A>;
	
	struct Node{
		using Type = NodeType;
		using Variant = NodeVariant;
		Variant node;
		std::vector<Node> branches;
		
		Type getType()const;
		bool operator==(const Node& nod)const;
	};
	
	
	std::string getNodeTypeName(Node::Type type);
	nlohmann::json printNodeVariant(const Node& nod);
	nlohmann::json printNode(const Node& nod);
	std::ostream& operator<<(std::ostream& out, const Node& nod);
	
	struct CSS{
		std::string data;
		
		bool operator==(const CSS& css)const;
	};
	
	struct PageTree{
		Node pageRoot;
		std::vector<CSS> cssData;
		std::vector<Code> codeData;
		
		bool operator==(const PageTree& page)const;
	};
	
	std::ostream& operator<<(std::ostream& out, const PageTree& page);
	
	PageTree makeTreeFromTokenedPage(TokenedPage tokenPage, ParserParameters parameters = {});
	PageTree makeTreeFromPage(std::string page, ParserParameters parameters = {});
	
	struct TreeContext{
        std::vector<Node> stack;
        std::vector<Node> styleCarry;
        int newlines;
        std::size_t tokenPos;
        TokenedPage tokenedPage;
        
        std::vector<CSS> cssData;
        std::vector<Code> codeData;
        
        ParserParameters parameters;
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
    
    void makeTop(TreeContext& context, Node::Type type);
}

#endif // TREER_HPP
