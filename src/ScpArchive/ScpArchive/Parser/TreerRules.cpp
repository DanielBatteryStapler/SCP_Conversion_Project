#include "TreerRules.hpp"

namespace Parser{
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
    
    void handlePlainText(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<PlainText>(token.token)});
    }
    
    void handleLiteralText(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<LiteralText>(token.token)});
    }
    
    void handleLineBreak(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<LineBreak>(token.token)});
    }
    
    void handleHyperLink(TreeContext& context, const Token& token){
        addAsText(context, Node{std::get<HyperLink>(token.token)});
    }
    
    void handleHeading(TreeContext& context, const Token& token){
        makeDivPushable(context);
        pushStack(context, Node{std::get<Heading>(token.token)});
    }
    
    void handleDivider(TreeContext& context, const Token& token){
        addAsDiv(context, Node{std::get<Divider>(token.token)});
    }
    
    namespace{
        void handleSectionStartEnd(const Token& token, std::function<void(const SectionStart&)> startFunc, std::function<void(const SectionEnd&)> endFunc){
            if(token.getType() == Token::Type::SectionStart){
                startFunc(std::get<SectionStart>(token.token));
            }
            else{
                endFunc(std::get<SectionEnd>(token.token));
            }
        }
    }
    
    void handleSpan(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{Span{section.parameters}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Span)){
                popSingle(context, Node::Type::Span);
            }
        });
    }
    
    void handleSize(TreeContext& context, const Token& token){
        handleSectionStartEnd(token, 
        [&](const SectionStart& section){
            makeTextAddable(context);
            pushStack(context, Node{Size{section.mainParameter}});
        }, [&](const SectionEnd& section){
            if(isInside(context, Node::Type::Size)){
                popSingle(context, Node::Type::Size);
            }
        });
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
}
