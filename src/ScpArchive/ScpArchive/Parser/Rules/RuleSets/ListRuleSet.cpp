#include "ListRuleSet.hpp"

namespace Parser{
	bool tryListPrefixRule(const TokenRuleContext& context){
        if(context.wasNewLine){
            std::size_t pos = context.pagePos;
			while(pos < context.page.size()){
				if(context.page[pos] == ' '){
					//keep going
				}
				else if(check(context.page, pos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
                    pos++;//the unicode space is 2 bytes long so we need to add in an extra increment
                    //keep going
				}
				else if(check(context.page, pos, "* ")){
                    return true;
				}
				else if(check(context.page, pos, "# ")){
                    return true;
				}
				else{
					return false;//fail if there is an unexpected character
				}
				pos++;
			}
        }
        return false;
	}
	
	TokenRuleResult doListPrefixRule(const TokenRuleContext& context){
        std::size_t pos = context.pagePos;
		ListPrefix prefix;
		prefix.degree = 1;
		while(pos < context.page.size()){
			if(context.page[pos] == ' '){
                prefix.degree++;
            }
            else if(check(context.page, pos, {static_cast<char>(0b11000010), static_cast<char>(0b10100000)})){//special unicode space
                pos++;//the unicode space is 2 bytes long so we need to add in an extra increment
                prefix.degree++;
            }
			else if(check(context.page, pos, "* ")){
				prefix.type = ListPrefix::Type::Bullet;
				pos += 2;//skip the "* "
				break;
			}
			else if(check(context.page, pos, "# ")){
                prefix.type = ListPrefix::Type::Number;
				pos += 2;//skip the "# "
				break;
			}
			pos++;
		}
		
		TokenRuleResult result;
        result.newPos = pos;
        result.newTokens.push_back(Token{prefix, context.pagePos, pos, context.page.substr(context.pagePos, pos - context.pagePos)});
        return result;
	}
}
