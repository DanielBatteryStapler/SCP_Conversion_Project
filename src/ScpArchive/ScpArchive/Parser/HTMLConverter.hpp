#ifndef HTMLCONVERTER_HPP
#define HTMLCONVERTER_HPP

#include "../HTTP/MarkupOutStream.hpp"
#include "Treer.hpp"

namespace Parser{
    struct HtmlContext{
        MarkupOutStream& out;
        mutable std::size_t uniqueId;
    };
    
    
	void toHtmlShownAuthor(MarkupOutStream& out, const ShownAuthor& author);
    
    void convertNodeToHtml(const HtmlContext& con, const Node& nod);
    void delegateNodeBranches(const HtmlContext& con, const Node& nod);
	std::string getUniqueHtmlId(const HtmlContext& con);
    
	void convertPageTreeToHtml(MarkupOutStream& out, const PageTree& tree);
	void convertTokenedPageToHtml(MarkupOutStream& out, const TokenedPage& page);
	
	std::string formatTimeStamp(TimeStamp timeStamp);
	std::string redirectLink(std::string rawLink);
}

#endif // HTMLCONVERTER_HPP
