#include "InlineFormatRuleSet.hpp"

#include <sstream>

namespace Parser{
    std::string toStringTokenInlineFormat(const TokenVariant& tok){
        std::stringstream ss;
        ss << "InlineFormat:";
        const InlineFormat& format = std::get<InlineFormat>(tok);
        switch(format.type){
            default:
                ss << "Unknown";
                break;
            case InlineFormat::Type::Bold:
                ss << "Bold";
                break;
            case InlineFormat::Type::Italics:
                ss << "Italics";
                break;
            case InlineFormat::Type::Strike:
                ss << "Strike";
                break;
            case InlineFormat::Type::Underline:
                ss << "Underline";
                break;
            case InlineFormat::Type::Super:
                ss << "Super";
                break;
            case InlineFormat::Type::Sub:
                ss << "Sub";
                break;
            case InlineFormat::Type::Monospace:
                ss << "Monospace";
                break;
            case InlineFormat::Type::Color:
                ss << "Color";
                break;
        }
        ss << "[" << (format.begin?"true":"false") << "," << (format.end?"true":"false") << "," << format.color << "]";
        return ss.str();
    }
    
    std::string toStringNodeStyleFormat(const NodeVariant& nod){
        std::stringstream ss;
        const StyleFormat& format = std::get<StyleFormat>(nod);
        ss << "StyleFormat:\"";
        switch(format.type){
            default:
                ss << "Unknown";
                break;
            case StyleFormat::Type::Bold:
                ss << "Bold";
                break;
            case StyleFormat::Type::Italics:
                ss << "Italics";
                break;
            case StyleFormat::Type::Strike:
                ss << "Strike";
                break;
            case StyleFormat::Type::Underline:
                ss << "Underline";
                break;
            case StyleFormat::Type::Super:
                ss << "Super";
                break;
            case StyleFormat::Type::Sub:
                ss << "Sub";
                break;
            case StyleFormat::Type::Color:
                ss << "Color," << format.color;
                break;
        }
        ss << "\"";
        return ss.str();
    }
	
	namespace{
		inline bool handleTryInlineFormatting(const TokenRuleContext& context, std::string format){
			if(!check(context.page, context.pagePos, format)){
				return false;
			}
			if(context.page.size() <= context.pagePos + format.size() || isspace(context.page[context.pagePos + format.size()])){
				if(0 > context.pagePos - 1 || isspace(context.page[context.pagePos - 1])){
					return false;
				}
			}
			return true;
		}
		
		inline TokenRuleResult handleDoInlineFormatting(const TokenRuleContext& context, InlineFormat::Type type, std::string format){
			
			InlineFormat token;
			token.type = type;
			if(context.page.size() <= context.pagePos + format.size() || isspace(context.page[context.pagePos + format.size()])){
				token.begin = false;
			}
			else{
				token.begin = true;
			}
			
			if(0 > context.pagePos - 1 || isspace(context.page[context.pagePos - 1])){
				token.end = false;
			}
			else{
				token.end = true;
			}
			
			const std::size_t begin = context.pagePos;
			const std::size_t end = context.pagePos + format.size();
			
			TokenRuleResult result;
			result.newPos = end;
			result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
			return result;
		}
	}
	
	bool tryStrikeRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "--");
	}
	TokenRuleResult doStrikeRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Strike, "--");
	}
	
	bool tryItalicsRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "//");
	}
	TokenRuleResult doItalicsRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Italics, "//");
	}
	
	bool tryBoldRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "**");
	}
	TokenRuleResult doBoldRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Bold, "**");
	}
	
	bool tryUnderlineRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "__");
	}
	TokenRuleResult doUnderlineRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Underline, "__");
	}
	
	bool trySuperRule(const TokenRuleContext& context){
		return handleTryInlineFormatting(context, "^^");
	}
	TokenRuleResult doSuperRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Super, "^^");
	}
	
	bool trySubRule(const TokenRuleContext& context){
		return !checkParagraph(context.page, context.pagePos, "''") && handleTryInlineFormatting(context, ",,");
	}
	TokenRuleResult doSubRule(const TokenRuleContext& context){
		return handleDoInlineFormatting(context, InlineFormat::Type::Sub, ",,");
	}
	
	bool tryMonospaceRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "{{")){
			return true;
		}
		else if(check(context.page, context.pagePos, "}}")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doMonospaceRule(const TokenRuleContext& context){
		InlineFormat token;
		token.type = InlineFormat::Type::Monospace;
		if(check(context.page, context.pagePos, "{{")){
			token.begin = true;
			token.end = false;
		}
		else{
			token.begin = false;
			token.end = true;
		}
		
		const std::size_t begin = context.pagePos;
		const std::size_t end = context.pagePos + 2;
		
		TokenRuleResult result;
		result.newPos = end;
		result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
		return result;
	}
	
	bool tryColorRule(const TokenRuleContext& context){
		if(check(context.page, context.pagePos, "##")){
			return true;
		}
		return false;
	}
	
	TokenRuleResult doColorRule(const TokenRuleContext& context){
		const std::size_t begin = context.pagePos;
		std::size_t end = context.pagePos;
		
		InlineFormat token;
		token.type = InlineFormat::Type::Color;
		if(checkLine(context.page, context.pagePos, "|")){
			token.begin = true;
			token.end = false;
			end += 2;
			while(context.page[end] != '|'){
				end++;
			}
			token.color = context.page.substr(begin + 2, end - begin - 2);
			token.color.erase(remove(token.color.begin(),token.color.end(),' '),token.color.end());//remove all whitespace
			end++;
		}
		else{
			end += 2;
			token.begin = false;
			token.end = true;
		}
		
		TokenRuleResult result;
		result.newPos = end;
		result.newTokens.push_back(Token{token, begin, end, context.page.substr(begin, end - begin)});
		return result;
	}
	
	void handleInlineFormat(TreeContext& context, const Token& token){
        const InlineFormat& tokenFormat = std::get<InlineFormat>(token.token);
        
        bool alreadyInStyle = false;
        for(const Node& i : context.stack){
            if(i.getType() == Node::Type::StyleFormat){
                const StyleFormat& format = std::get<StyleFormat>(i.node);
                if(format.type == tokenFormat.type){
                    alreadyInStyle = true;
                    break;
                }
            }
        }
        if(alreadyInStyle && tokenFormat.end){
            popSingle(context, [tokenFormat](const Node& nod){
                if(nod.getType() == Node::Type::StyleFormat){
                    const StyleFormat& format = std::get<StyleFormat>(nod.node);
                    if(tokenFormat.type == format.type){
                        return true;
                    }
                }
                return false;
            });
            return;
        }
        else if(!alreadyInStyle && tokenFormat.begin){
            //there needs to be a valid end token too, so let's check for that
            bool hasValidEnd = false;
            {
                std::size_t pos = context.tokenPos + 1;
                bool wasNewline = false;
                while(pos < context.tokenedPage.tokens.size()){
                    const Token& checkToken = context.tokenedPage.tokens[pos];
                    if(checkToken.getType() == Token::Type::InlineFormat){
                        const InlineFormat& inlineFormat = std::get<InlineFormat>(checkToken.token);
                        if(tokenFormat.type == inlineFormat.type && inlineFormat.end){
                            hasValidEnd = true;
                            break;
                        }
                    }
                    else if(checkToken.getType() == Token::Type::NewLine){
                        if(wasNewline){
                            break;
                        }
                        else{
                            wasNewline = true;
                        }
                    }
                    else{
                        wasNewline = false;
                    }
                    pos++;
                }
            }
            if(hasValidEnd){
                makeTextAddable(context);
                pushStack(context, Node{StyleFormat{tokenFormat.type, tokenFormat.color}});
                return;
            }
        }
        //this format marker doesn't line up with starting or stopping anything, so that means it is
        //"degenerate"(in the mathematical sense) and should go back into plain text
        switch(tokenFormat.type){
            default:
                addAsText(context, Node{PlainText{token.source}});
                break;
            case InlineFormat::Type::Strike:
                addAsText(context, Node{PlainText{"—"}});
                break;
        }
    }
    
        void toHtmlNodeStyleFormat(const HtmlContext& con, const Node& nod){
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
        con.out << "<"_AM << allowMarkup(startTag) << ">"_AM;
        delegateNodeBranches(con, nod);
        con.out << "</"_AM << allowMarkup(endTag) << ">"_AM;
	}
}