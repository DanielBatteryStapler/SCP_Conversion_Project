#ifndef HTMLCONVERTER_HPP
#define HTMLCONVERTER_HPP

#include "../HTTP/MarkupOutStream.hpp"
#include "Treer.hpp"

namespace Parser{
	void convertPageTreeToHtml(MarkupOutStream& out, const PageTree& tree);
	void convertTokenedPageToHtml(MarkupOutStream& out, const TokenedPage& page);
};

#endif // HTMLCONVERTER_HPP
