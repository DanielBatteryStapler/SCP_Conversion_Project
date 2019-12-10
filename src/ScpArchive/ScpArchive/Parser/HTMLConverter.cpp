#include "HTMLConverter.hpp"

namespace Parser{
	namespace{
		
		void delegateBranch(MarkupOutStream& out, const Node& nod);
		
		template<typename T>
		void handleNode(MarkupOutStream& out, const Node& nod){
            throw std::runtime_error("Attempted to run a node handle for a node with no handle defined");
		}
		
		template<>
		void handleNode<HyperLink>(MarkupOutStream& out, const Node& nod){
            const HyperLink& node = std::get<HyperLink>(nod.node);
            out << "<a href='"_AM << node.url << "'>"_AM << node.shownText << "</a>"_AM; 
		}
		
		template<>
		void handleNode<LineBreak>(MarkupOutStream& out, const Node& nod){
            const LineBreak& node = std::get<LineBreak>(nod.node);
            out << "<br />"_AM; 
		}
		
		template<>
		void handleNode<LiteralText>(MarkupOutStream& out, const Node& nod){
            const LiteralText& node = std::get<LiteralText>(nod.node);
            out << node.text;///TODO: this isn't correct, it should have a style that makes some whitespace oddities 
		}
		
		template<>
		void handleNode<Paragraph>(MarkupOutStream& out, const Node& nod){
            const Paragraph& node = std::get<Paragraph>(nod.node);
            out << "<p>"_AM;
            delegateBranch(out, nod);
            out << "</p>"_AM;
		}
		
		template<>
		void handleNode<PlainText>(MarkupOutStream& out, const Node& nod){
            const PlainText& node = std::get<PlainText>(nod.node);
            out << node.text;
		}
		
		template<>
		void handleNode<RootPage>(MarkupOutStream& out, const Node& nod){
            const RootPage& node = std::get<RootPage>(nod.node);
            delegateBranch(out, nod);
		}
		
		template<>
		void handleNode<StyleFormat>(MarkupOutStream& out, const Node& nod){
            const StyleFormat& node = std::get<StyleFormat>(nod.node);
            ///TODO: StyleFormat: update to HTML5
            std::string startTag;
            std::string endTag;
            switch(node.type){
			default:
				throw std::runtime_error("Invalid StyleFormat::Type");
				break;
			case StyleFormat::Type::Bold:
				startTag = "b";
				endTag = "b";
				break;
			case StyleFormat::Type::Italics:
				startTag = "i";
				endTag = "i";
				break;
			case StyleFormat::Type::Strike:
				startTag = "strike";
				endTag = "strike";
				break;
			case StyleFormat::Type::Sub:
				startTag = "sub";
				endTag = "sub";
				break;
			case StyleFormat::Type::Super:
				startTag = "sup";
				endTag = "sup";
				break;
			case StyleFormat::Type::Underline:
				startTag = "u";
				endTag = "u";
				break;
			case StyleFormat::Type::Monospace:
				startTag = "span class='Monospaced'";
				endTag = "span";
				break;
			case StyleFormat::Type::Color:
				startTag = "span style='color:" + node.color + ";'";
				endTag = "span";
				break;
            }
            out << "<"_AM << allowMarkup(startTag) << ">"_AM;
            delegateBranch(out, nod);
            out << "</"_AM << allowMarkup(endTag) << ">"_AM;
		}
		
		template<>
		void handleNode<Heading>(MarkupOutStream& out, const Node& nod){
            const Heading& node = std::get<Heading>(nod.node);
            out << "<h"_AM << std::to_string(node.degree) << ">"_AM;
            delegateBranch(out, nod);
			out << "</h"_AM << std::to_string(node.degree) << ">"_AM;
		}
		
		template<>
		void handleNode<QuoteBox>(MarkupOutStream& out, const Node& nod){
            const QuoteBox& node = std::get<QuoteBox>(nod.node);
            out << "<blockquote>"_AM;
            delegateBranch(out, nod);
			out << "</blockquote>"_AM;
		}
		
		template<>
		void handleNode<ListElement>(MarkupOutStream& out, const Node& nod){
            const ListElement& node = std::get<ListElement>(nod.node);
            out << "<li>"_AM;
            delegateBranch(out, nod);
			out << "</li>"_AM;
		}
		
		template<>
		void handleNode<List>(MarkupOutStream& out, const Node& nod){
            const List& node = std::get<List>(nod.node);
            
            switch(node.type){
				case List::Type::Bullet:
					out << "<ul>"_AM;
					delegateBranch(out, nod);
					out << "</ul>"_AM;
					break;
				case List::Type::Number:
					out << "<ol>"_AM;
					delegateBranch(out, nod);
					out << "</ol>"_AM;
					break;
				default:
					throw std::runtime_error("Invalid List type");
            }
		}
		
		template<>
		void handleNode<Divider>(MarkupOutStream& out, const Node& nod){
            const Divider& node = std::get<Divider>(nod.node);
            switch(node.type){
				case Divider::Type::Line:
					out << "<hr />"_AM;
					break;
				case Divider::Type::Clear:
					out << "<div class='PageClearer' />"_AM;
					break;
				default:
					throw std::runtime_error("Invalid Divider type");
            }
		}
		
		template<>
		void handleNode<Size>(MarkupOutStream& out, const Node& nod){
            const Size& node = std::get<Size>(nod.node);
            out << "<span style='font-size:"_AM << node.size << "'>"_AM;
            delegateBranch(out, nod);
            out << "</span>"_AM;
		}
		
		template<>
		void handleNode<Span>(MarkupOutStream& out, const Node& nod){
            const Span& node = std::get<Span>(nod.node);
            out << "<span"_AM;
            for(auto i = node.parameters.begin(); i != node.parameters.end(); i++){
                out << " "_AM << i->first << "='"_AM << i->second << "'"_AM;
            }
            out << ">"_AM;
            delegateBranch(out, nod);
            out << "</span>"_AM;
		}
		
		template<>
		void handleNode<Align>(MarkupOutStream& out, const Node& nod){
            const Align& node = std::get<Align>(nod.node);
            std::string textAlign;
            switch(node.type){
                case Align::Type::Center:
                    textAlign = "center";
                    break;
                case Align::Type::Left:
                    textAlign = "left";
                    break;
                case Align::Type::Right:
                    textAlign = "right";
                    break;
                case Align::Type::Justify:
                    textAlign = "justify";
                    break;
                default:
                    throw std::runtime_error("Encountered invalid Align::Type");
            }
            out << "<div style='text-align:"_AM << textAlign << "'>"_AM;
            delegateBranch(out, nod);
            out << "</div>"_AM;
		}
		
		void delegateNode(MarkupOutStream& out, const Node& nod){
			switch(nod.getType()){
                default:
                    throw std::runtime_error("Attempted to render TreePage with an Unknown/Invalid Node");
                case Node::Type::HyperLink:
                    handleNode<HyperLink>(out, nod);
                    break;
                case Node::Type::LineBreak:
                    handleNode<LineBreak>(out, nod);
                    break;
                case Node::Type::LiteralText:
                    handleNode<LiteralText>(out, nod);
                    break;
                case Node::Type::Paragraph:
                    handleNode<Paragraph>(out, nod);
                    break;
                case Node::Type::PlainText:
                    handleNode<PlainText>(out, nod);
                    break;
                case Node::Type::RootPage:
                    handleNode<RootPage>(out, nod);
                    break;
                case Node::Type::StyleFormat:
                    handleNode<StyleFormat>(out, nod);
                    break;
				case Node::Type::Heading:
					handleNode<Heading>(out, nod);
					break;
				case Node::Type::QuoteBox:
					handleNode<QuoteBox>(out, nod);
					break;
				case Node::Type::List:
					handleNode<List>(out, nod);
					break;
				case Node::Type::ListElement:
					handleNode<ListElement>(out, nod);
					break;
				case Node::Type::Divider:
					handleNode<Divider>(out, nod);
					break;
                case Node::Type::Size:
                    handleNode<Size>(out, nod);
                    break;
                case Node::Type::Span:
                    handleNode<Span>(out, nod);
                    break;
                case Node::Type::Align:
                    handleNode<Align>(out, nod);
			}
		}
		
		void delegateBranch(MarkupOutStream& out, const Node& nod){
			for(const Node& i : nod.branches){
				delegateNode(out, i);
			}
		}
	}
	
	void convertPageTreeToHtml(MarkupOutStream& out, const PageTree& tree){
		delegateNode(out, tree.pageRoot);
	}
	
	namespace{
		std::array<std::string, 50> colors = { 
			"FF0000", "00FF00", "0000FF", "FFFF00", "FF00FF", "00FFFF",
			"800000", "008000", "000080", "808000", "800080", "008080",
			"C00000", "00C000", "0000C0", "C0C000", "C000C0", "00C0C0",
			"400000", "004000", "000040", "404000", "400040", "004040",
			"200000", "002000", "000020", "202000", "200020", "002020",
			"600000", "006000", "000060", "606000", "600060", "006060",
			"A00000", "00A000", "0000A0", "A0A000", "A000A0", "00A0A0",
			"E00000", "00E000", "0000E0", "E0E000", "E000E0", "00E0E0"
		};//just some basic colors to make the annotated html look nice
		//thank you, https://stackoverflow.com/questions/309149/generate-distinctly-different-rgb-colors-in-graphs#309193
		std::string getColor(int i, bool hover){
			constexpr auto toNum = [](char c)->int{
				if(std::isdigit(c)){
					return c - 'A' + 10;
				}
				else{
					return c - '0';
				}
			};
			std::string raw = colors[i % colors.size()];
			
			int r = toNum(raw[0]) * 16 + toNum(raw[1]);
			int g = toNum(raw[2]) * 16 + toNum(raw[3]);
			int b = toNum(raw[4]) * 16 + toNum(raw[5]);
			if(hover){
				return "rgba("  + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", 0.5)";
			}
			else{
				return "rgba("  + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", 0.25)";
			}
		}
	};
	
	void convertTokenedPageToHtml(MarkupOutStream& out, const TokenedPage& page){
		std::size_t pos = 0; 
		auto tok = page.tokens.begin();
		
		while(pos < page.originalPage.size()){
			if(tok != page.tokens.end() && pos == tok->sourceStart){
				out << "<span style='background-color:"_AM << getColor(tok->token.index(), false) << ";' "_AM
				<< " onMouseOver='this.style.backgroundColor=\""_AM << getColor(tok->token.index(), true) << "\"'"_AM
				<< " onMouseOut='this.style.backgroundColor=\""_AM << getColor(tok->token.index(), false) << "\"'"_AM
				<< " title='"_AM << tokenVariantToString(*tok) << "'>"_AM;
				for(char c : tok->source){
					if(c == '\n'){
						out << "\u00B6<br />"_AM;
					}
					else{
						out << c;
					}
				}
				out << "</span>"_AM;
				pos = tok->sourceEnd;
				tok++;
			}
			else{
				if(page.originalPage[pos] == '\n'){
					out << "\u00B6<br />"_AM;
				}
				else{
					out << page.originalPage[pos];
				}
				pos++;
			}
		}
		
	}
	
	
	
	
	
	
}


