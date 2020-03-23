#ifndef TEMPLATER_HPP
#define TEMPLATER_HPP

#include "Parser.hpp"

namespace Parser{
    std::string applyPageTemplate(std::string format, std::string pageContent, PageInfo pageInfo = {});
    std::string applyIncludeParameters(std::string pageContent, const std::map<std::string, std::string>& parameters);
}

#endif // TEMPLATER_HPP
