
#include <cbeditor.h>
#include <editormanager.h>
#include <cbstyledtextctrl.h>

#include "stringio.h"
#include "astnodeutil.h"
#include "clangcclogger.h"


namespace ASTNode
{

const Decl* GetDeclaration(const NodeType& node)
{
    switch(node.which())
    {
        case 0:
            return nullptr;
        case 1:
            return boost::get<Decl*>(node);
        case 2:
            return GetDeclarationFromStatement(boost::get<Stmt*>(node));
        case 3:
            return GetDeclarationFromTypeLoc(boost::get<TypeLoc>(node));
        case 4:
            return GetDeclarationFromRefNode(boost::get<RefNode>(node));
    }
}
//Todo Add as necessary.
const Decl* GetDeclarationFromStatement(const Stmt* stmt)
{
    if (const DeclRefExpr* expr = dyn_cast<DeclRefExpr>(stmt))
        return expr->getDecl();
    else if (const MemberExpr* expr = dyn_cast<MemberExpr>(stmt))
        return expr->getMemberDecl();
    else if (const CXXConstructExpr* expr = dyn_cast<CXXConstructExpr>(stmt))
        return expr->getConstructor();
    return nullptr;
}
const Decl* GetDeclarationFromType(const QualType& type)
{
    if (type.isNull())
    {
        LoggerAccess::Get()->Log("null qual type in GetDeclarationFrom type");
        return nullptr;
    }

    if(const TagType* ttype = type->getAs<TagType>())
        return ttype->getDecl();
    return nullptr;

}
//Add as necessary
const Decl* GetDeclarationFromTypeLoc(const TypeLoc& tloc)
{
    if (TagTypeLoc typeLoc = tloc.getAs<TagTypeLoc>())
        return typeLoc.getDecl();
    else if (TypedefTypeLoc typeLoc = tloc.getAs<TypedefTypeLoc>())
        return typeLoc.getTypedefNameDecl();
    else if (InjectedClassNameTypeLoc typeLoc = tloc.getAs<InjectedClassNameTypeLoc>())
        return typeLoc.getDecl();
    else if (UnresolvedUsingTypeLoc typeLoc = tloc.getAs<UnresolvedUsingTypeLoc>())
        return typeLoc.getDecl();
    else if (TemplateTypeParmTypeLoc typeLoc = tloc.getAs<TemplateTypeParmTypeLoc>())
        return typeLoc.getDecl();
    else if (TemplateSpecializationTypeLoc typeLoc = tloc.getAs<TemplateSpecializationTypeLoc>())
        return typeLoc.getTypePtr()->getTemplateName().getAsTemplateDecl();
    //FIXME this does not work;
    else if (AutoTypeLoc typeLoc = tloc.getAs<AutoTypeLoc>())
        return GetDeclarationFromType(typeLoc.getTypePtr()->getDeducedType());

    return nullptr;
}


const Decl* GetDeclarationFromRefNode(const RefNode& refNode)
{
    return refNode.GetReferencedDeclaration();
}

const Decl* GetDefinition(const Decl* decl)
{
    if (const TagDecl* tagDecl = dyn_cast<TagDecl>(decl))
        return tagDecl->getDefinition();

    if (const FunctionDecl* funcDecl = dyn_cast<FunctionDecl>(decl))
    {
        const FunctionDecl* definition = 0;
        funcDecl->getBody(definition);
        return definition;
    }

    return nullptr;
}

TemplateDecl* GetDescribedTemplate(const Decl* decl)
{
    if (const CXXRecordDecl* classDecl = dyn_cast<CXXRecordDecl>(decl))
        return classDecl->getDescribedClassTemplate();
    if (const FunctionDecl* funcDecl = dyn_cast<FunctionDecl>(decl))
        return funcDecl->getDescribedFunctionTemplate();
    return nullptr;

}

void GotoDeclarationInEditor(const Decl* decl)
{
    auto& srcMgr = decl->getASTContext().getSourceManager();
    auto loc = decl->getLocation();
    auto expansionLoc = srcMgr.getExpansionLoc(loc);
    auto fileAndOffset = srcMgr.getDecomposedLoc(expansionLoc);
    std::string fileName = srcMgr.getFilename(expansionLoc);

    cbEditor* ed = Manager::Get()->GetEditorManager()->Open(std2wx(fileName));
    if (!ed)
        return;
    ed->GetControl()->GotoPos(fileAndOffset.second);
    ed->SetFocus();

}
SourceRange GetDeclarationRange(const Decl* decl)
{
    auto& srcMgr = decl->getASTContext().getSourceManager();
    auto& langOpts = decl->getASTContext().getLangOpts();
    auto invalidRange = std::make_pair(-1,-1);
    SourceRange range;
    // if this declaration describes a template
    if (TemplateDecl* templateDecl = GetDescribedTemplate(decl))
        range = templateDecl->getSourceRange();
    else
        range = decl->getSourceRange();

    //if the next token is a semicolon we need to select that as well.
    //findlocationAfterToken() advances to the end of the token automatically.
    SourceLocation endLoc = Lexer::findLocationAfterToken(range.getEnd(), tok::semi,
                                                          srcMgr, langOpts, false);
    if (endLoc.isInvalid())
    //End of the source range points to the start of the last token but we
    //need the one past the end for selection.
        endLoc= Lexer::getLocForEndOfToken(range.getEnd(), 0, srcMgr, langOpts);

    range.setEnd(endLoc);
    return range;
}
void SelectDeclarationInEditor(const Decl* decl)
{
    auto& srcMgr = decl->getASTContext().getSourceManager();
    SourceRange range = GetDeclarationRange(decl);

    auto startFileAndOffset = srcMgr.getDecomposedExpansionLoc(range.getBegin());
    auto endFileAndOffset = srcMgr.getDecomposedExpansionLoc(range.getEnd());

    std::string fileName = srcMgr.getFilename(range.getBegin());

    cbEditor* ed = Manager::Get()->GetEditorManager()->Open(std2wx(fileName));
    if (!ed)
        return;
    ed->GetControl()->SetSelectionVoid(startFileAndOffset.second, endFileAndOffset.second);
}

}
