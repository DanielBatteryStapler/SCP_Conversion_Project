#include "ImageRuleSet.hpp"

#include <sstream>

namespace Parser{
	std::string toStringNodeImage(const NodeVariant& nod){
        const Image& image = std::get<Image>(nod);
        std::stringstream ss;
        ss << "Image:" << image.source << "," << (image.newWindow?"true":"false") << "," << image.link << ","
        << image.alt << "," << image.title << "," << image.width << "," << image.height << ","
        << image.style << "," << image.cssClass << ",";
        switch(image.alignment){
        case Image::AlignmentType::Default:
            ss << "Default";
            break;
        case Image::AlignmentType::Center:
            ss << "Center";
            break;
        case Image::AlignmentType::Left:
            ss << "Left";
            break;
        case Image::AlignmentType::Right:
            ss << "Right";
            break;
        case Image::AlignmentType::FloatLeft:
            ss << "FloatLeft";
            break;
        case Image::AlignmentType::FloatRight:
            ss << "FloatRight";
            break;
        }
        return ss.str();
	}
	
    void handleImage(TreeContext& context, const Token& token){
        const Section& section = std::get<Section>(token.token);
        
        Image newImage;
        if(check(section.typeString, 0, "=")){
            newImage.alignment = Image::AlignmentType::Center;
        }
        else if(check(section.typeString, 0, "<")){
            newImage.alignment = Image::AlignmentType::Left;
        }
        else if(check(section.typeString, 0, ">")){
            newImage.alignment = Image::AlignmentType::Right;
        }
        else if(check(section.typeString, 0, "f<")){
            newImage.alignment = Image::AlignmentType::FloatLeft;
        }
        else if(check(section.typeString, 0, "f>")){
            newImage.alignment = Image::AlignmentType::FloatRight;
        }
        else{
            newImage.alignment = Image::AlignmentType::Default;
        }
        
        if(section.parameters.find("link") != section.parameters.end()){
            newImage.link = section.parameters.find("link")->second;
            if(check(newImage.link, 0, "*")){
                newImage.newWindow = true;
                newImage.link = newImage.link.substr(1, newImage.link.size() - 1);
            }
            else{
                newImage.newWindow = false;
            }
        }
        else{
            newImage.link = "";
            newImage.newWindow = false;
        }
        if(section.parameters.find("alt") != section.parameters.end()){
            newImage.alt = section.parameters.find("alt")->second;
        }
        if(section.parameters.find("title") != section.parameters.end()){
            newImage.title = section.parameters.find("title")->second;
        }
        if(section.parameters.find("width") != section.parameters.end()){
            newImage.width = section.parameters.find("width")->second;
        }
        if(section.parameters.find("height") != section.parameters.end()){
            newImage.height = section.parameters.find("height")->second;
        }
        if(section.parameters.find("style") != section.parameters.end()){
            newImage.style = section.parameters.find("style")->second;
        }
        if(section.parameters.find("class") != section.parameters.end()){
            newImage.cssClass = section.parameters.find("class")->second;
        }
        
        std::string source = section.mainParameter;
        if(check(source, 0, "http://") || check(source, 0, "https://")){
            newImage.source = source;
        }
        else if(check(source, 0, "/") && source.find("/", 1) != std::string::npos){
            newImage.source = "__system/pageFile" + source;
        }
        else{
            newImage.source = "__system/pageFile/" + context.parameters.pageName + "/" + urlEncode(source);
        }
        
        addAsDiv(context, Node{newImage});
    }
	
	void toHtmlNodeImage(const HtmlContext& con, const Node& nod){
        const Image& image = std::get<Image>(nod.node);
        if(image.alignment != Image::AlignmentType::Default){
            con.out << "<div class='"_AM;
            switch(image.alignment){
                case Image::AlignmentType::Center:
                    con.out << "ImageCenter";
                    break;
                case Image::AlignmentType::Left:
                    con.out << "ImageLeft";
                    break;
                case Image::AlignmentType::Right:
                    con.out << "ImageRight";
                    break;
                case Image::AlignmentType::FloatLeft:
                    con.out << "ImageFloatLeft";
                    break;
                case Image::AlignmentType::FloatRight:
                    con.out << "ImageFloatRight";
                    break;
            }
            con.out << "'>"_AM;
        }
        
        if(image.link != ""){
            con.out << "<a href='"_AM << image.link << "'>"_AM;
        }
        
        con.out << "<img src='"_AM << image.source << "'"_AM;
        if(image.alt != ""){
            con.out << " alt='"_AM << image.alt << "'"_AM;
        }
        if(image.title != ""){
            con.out << " title='"_AM << image.title << "'"_AM;
        }
        if(image.height != ""){
            con.out << " height='"_AM << image.height << "'"_AM;
        }
        if(image.width != ""){
            con.out << " width='"_AM << image.width << "'"_AM;
        }
        if(image.style != ""){
            con.out << " style='"_AM << image.style << "'"_AM;
        }
        if(image.cssClass != ""){
            con.out << " class='"_AM << image.cssClass << "'"_AM;
        }
        else{
            con.out << " class='image'"_AM;
        }
        con.out << " />"_AM;
        
        if(image.link != ""){
            con.out << "</a>"_AM;
        }
        
        if(image.alignment != Image::AlignmentType::Default){
            con.out << "</div>"_AM;
        }
	}
}
