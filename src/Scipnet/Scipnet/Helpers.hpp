#ifndef HELPERS_HPP_INCLUDED
#define HELPERS_HPP_INCLUDED

#include <string>

static std::string redirectLink(std::string rawLink){
	std::string output;
	const auto doRedirect = [&rawLink, &output](std::string oldPrefix, std::string newPrefix){
		if(output == "" && rawLink.rfind(oldPrefix, 0) == 0){
			output = newPrefix + rawLink.substr(oldPrefix.size(), rawLink.size() - oldPrefix.size());
		}
	};
	doRedirect("http://www.scp-wiki.net/local--files/", "/__system/pageFile/");
	doRedirect("http://www.scp-wiki.net/", "/");
	doRedirect("http://scp-wiki.wikidot.com/", "/");
	doRedirect("http://scp-wiki.wdfiles.com/local--files/", "/__system/pageFile/");
	if(output == ""){
		return rawLink;
	}
	else{
		return output;
	}
}

#endif // HELPERS_HPP_INCLUDED
