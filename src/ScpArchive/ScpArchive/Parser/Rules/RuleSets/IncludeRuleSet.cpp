#include "IncludeRuleSet.hpp"

#include "../../../Database/Database.hpp"

namespace Parser{
    void handleInclude(TreeContext& context, const Token& token){
        if(context.parameters.includeDepth > 5){//if we have gone too many layers deep, then we need to stop doing this
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Include failed: maximum include depth reached"}});
            makeDivPushable(context);
            return;
        }
        
        const Section& section = std::get<Section>(token.token);
        auto pageName = section.mainParameter;
        auto db = context.parameters.database;
        if(db == nullptr){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Include failed: database connection is invalid"}});
            makeDivPushable(context);
            return;
        }
        if(check(pageName, 0, ":scp-wiki:")){//if you're requesting a page on the current site, there's no need for a prefix
            const std::size_t length = std::string(":scp-wiki:").size();
            pageName = pageName.substr(length, pageName.size() - length);
        }
        if(pageName == context.parameters.page.name){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Include failed: recursive include is not allowed"}});
            makeDivPushable(context);
            return;
        }
        std::optional<Database::ID> pageId = db->getPageId(pageName);
        if(!pageId){
            makeDivPushable(context);
            addAsText(context, Node{PlainText{"Include failed: cannot find page \"" + pageName + "\""}});
            makeDivPushable(context);
            return;
        }
        Database::PageRevision pageRevision = db->getLatestPageRevision(*pageId);
        ParserParameters parameters = context.parameters;
        parameters.includeParameters = section.parameters;
        parameters.includeDepth = context.parameters.includeDepth + 1;
        PageTree pageTree = Parser::makeTreeFromPage(pageRevision.sourceCode, parameters);
        
        makeDivPushable(context);
        //so what this line does is get all of the nodes from the pageTree and insert them into the current page
        //appending vectors is really ugly in C++ so this is the beautiful line we get
        context.stack.back().branches.insert(context.stack.back().branches.end(), pageTree.pageRoot.branches.begin(), pageTree.pageRoot.branches.end());
        context.cssData.insert(context.cssData.begin(), pageTree.cssData.begin(), pageTree.cssData.end());
    }
}
