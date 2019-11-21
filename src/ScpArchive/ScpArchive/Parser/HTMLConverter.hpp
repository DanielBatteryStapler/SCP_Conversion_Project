#ifndef HTMLCONVERTER_HPP
#define HTMLCONVERTER_HPP

#include "../HTTP/MarkupOutStream.hpp"
#include "Treer.hpp"

namespace Parser{
	void convertPageTreeToHtml(MarkupOutStream& out, const PageTree& tree);
};

#endif // HTMLCONVERTER_HPP
