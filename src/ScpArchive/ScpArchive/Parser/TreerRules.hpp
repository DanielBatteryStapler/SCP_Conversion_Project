#ifndef TREERRULES_HPP
#define TREERRULES_HPP

#include "Treer.hpp"

namespace Parser{
    void handleInlineFormat(TreeContext& context, const Token& token);
    void handlePlainText(TreeContext& context, const Token& token);
    void handleLiteralText(TreeContext& context, const Token& token);
    void handleLineBreak(TreeContext& context, const Token& token);
    void handleHyperLink(TreeContext& context, const Token& token);
    void handleInlineFormat(TreeContext& context, const Token& token);
    void handleHeading(TreeContext& context, const Token& token);
    void handleDivider(TreeContext& context, const Token& token);
    
    void handleSpan(TreeContext& context, const Token& token);
    void handleSize(TreeContext& context, const Token& token);
}

#endif // TREERRULES_HPP
