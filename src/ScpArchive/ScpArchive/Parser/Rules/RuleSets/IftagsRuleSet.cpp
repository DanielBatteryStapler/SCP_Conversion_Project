#include "IftagsRuleSet.hpp"

namespace Parser{
    void handleIftags(TreeContext& context, const Token& token){
        handleSectionStartEnd(token,
        [&](const SectionStart& section){
            
            //this is a really weird tag
            bool condition = true;
            for(auto i = section.parameters.begin(); i != section.parameters.end(); i++){
                std::string tag = i->first;
                if(tag.size() > 0){
                    switch(tag[0]){
                        case '-':
                            tag = tag.substr(1, tag.size() - 1);
                            if(std::find(context.parameters.pageTags.begin(), context.parameters.pageTags.end(), tag) != context.parameters.pageTags.end()){
                                condition = false;
                            }
                            break;
                        case '+':
                            tag = tag.substr(1, tag.size() - 1);
                        default:
                            if(std::find(context.parameters.pageTags.begin(), context.parameters.pageTags.end(), tag) == context.parameters.pageTags.end()){
                                condition = false;
                            }
                            break;
                    }
                }
            }
            
            if(!condition){
                std::size_t pos = context.tokenPos;
                int depth = 0;
                while(pos < context.tokenedPage.tokens.size()){
                    if(context.tokenedPage.tokens[pos].getType() == Token::Type::SectionEnd){
                        const SectionEnd& section = std::get<SectionEnd>(context.tokenedPage.tokens[pos].token);
                        if(section.type == SectionType::Iftags){
                            depth--;
                            if(depth == 0){
                                context.tokenPos = pos;
                                break;
                            }
                        }
                    }
                    else if(context.tokenedPage.tokens[pos].getType() == Token::Type::SectionStart){
                        const SectionStart& section = std::get<SectionStart>(context.tokenedPage.tokens[pos].token);
                        if(section.type == SectionType::Iftags){
                            depth++;
                        }
                    }
                    pos++;
                }
            }
            
        }, [&](const SectionEnd& section){
            //do absolutely nothing
        });
    }
}
