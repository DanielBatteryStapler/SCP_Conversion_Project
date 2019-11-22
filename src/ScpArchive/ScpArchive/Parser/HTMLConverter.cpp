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
            out << "<quotebox>"_AM;
            delegateBranch(out, nod);
			out << "</quotebox>"_AM;
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
}
