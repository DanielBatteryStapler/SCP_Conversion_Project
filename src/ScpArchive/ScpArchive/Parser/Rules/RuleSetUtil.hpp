#ifndef RULEFUNCTIONS_HPP
#define RULEFUNCTIONS_HPP

#include <string>
#include <algorithm>

#include "RuleSet.hpp"

namespace Parser{
    inline static bool check(const std::string& buffer, std::size_t pos, std::string text){
        if(pos + text.size() > buffer.size()){
            return false;
        }
        std::string temp = buffer.substr(pos, text.size());
        return text == temp;
    }
    
    inline static bool checkWithoutCase(const std::string& buffer, std::size_t pos, std::string text){
        if(pos + text.size() > buffer.size()){
            return false;
        }
        std::string temp = buffer.substr(pos, text.size());
        std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        return text == temp;
    }

    inline static bool checkLine(const std::string& buffer, std::size_t pos, std::string text){
        while(pos < buffer.size()){
            if(check(buffer, pos, text)){
                return true;
            }
            if(check(buffer, pos, "\n")){
                return false;
            }
            pos++;
        }
        return false;
    }

    inline static bool checkParagraph(const std::string& buffer, std::size_t pos, std::string text){
        while(pos < buffer.size()){
            if(check(buffer, pos, text)){
                return true;
            }
            if(check(buffer, pos, "\n\n")){
                return false;
            }
            pos++;
        }
        return false;
    }

    inline static bool checkParagraphBack(const std::string& buffer, std::size_t pos, std::string text){
        while(true){
            if(check(buffer, pos, text)){
                return true;
            }
            if(check(buffer, pos, "\n\n")){
                return false;
            }
            if(pos == 0){
                break;
            }
            pos--;
        }
        return false;
    }
    
    inline static void handleSectionStartEnd(const Token& token, std::function<void(const SectionStart&)> startFunc, std::function<void(const SectionEnd&)> endFunc){
        if(token.getType() == Token::Type::SectionStart){
            startFunc(std::get<SectionStart>(token.token));
        }
        else{
            endFunc(std::get<SectionEnd>(token.token));
        }
    }
    
    inline static void travelPageTreeNodes(Node& root, std::function<bool(Node& node)> func){
        if(func(root)){
            for(Node& branch : root.branches){
                travelPageTreeNodes(branch, func);
            }
        }
    }
}

#endif // RULEFUNCTIONS_HPP
