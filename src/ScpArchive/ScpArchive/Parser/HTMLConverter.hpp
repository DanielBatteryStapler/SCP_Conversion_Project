#ifndef HTMLCONVERTER_HPP
#define HTMLCONVERTER_HPP

#include "../HTTP/MarkupOutStream.hpp"
#include "Treer.hpp"

namespace Parser{
    struct HtmlContext{
        MarkupOutStream& out;
        mutable std::size_t uniqueId;
    };
    
    void convertNodeToHtml(const HtmlContext& con, const Node& nod);
    void delegateNodeBranches(const HtmlContext& con, const Node& nod);
	std::string getUniqueHtmlId(const HtmlContext& con);
	void convertPageTreeToHtml(MarkupOutStream& out, const PageTree& tree);
	
	void convertTokenedPageToHtmlAnnotations(MarkupOutStream& out, const TokenedPage& page);
	void convertNodeToHtmlAnnotations(MarkupOutStream& out, const Node& nod);
	void convertPageTreeToHtmlAnnotations(MarkupOutStream& out, const PageTree& page);
	
	void toHtmlShownAuthor(MarkupOutStream& out, const ShownAuthor& author);
	std::string formatTimeStamp(TimeStamp timeStamp);
	std::string redirectLink(std::string rawLink);
}

#endif // HTMLCONVERTER_HPP
