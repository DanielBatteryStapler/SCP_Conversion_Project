#ifndef TEMPLATER_HPP
#define TEMPLATER_HPP

#include "Parser.hpp"

namespace Parser{
    std::string applyPageTemplate(std::string format, std::string page, PageInfo pageInfo = {});
    std::string applyIncludeParameters(std::string page, const std::map<std::string, std::string>& parameters);
}

#endif // TEMPLATER_HPP
