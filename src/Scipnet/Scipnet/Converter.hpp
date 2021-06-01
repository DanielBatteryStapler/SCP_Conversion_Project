#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include "layout/Layout.hpp"

#include "nlohmann/json.hpp"

namespace Converter{
	std::shared_ptr<Element> makeLayoutFromPageJson(const nlohmann::json& page);
	
	inline int maxPageWidth = 140;
	inline int imageMaxWidthExpandCutoff = 100;
};

#endif // CONVERTER_HPP
