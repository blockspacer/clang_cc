#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <wx/string.h>
#include "filefilters.h"
#include <clang/AST/Decl.h>
#include <clang/Sema/CodeCompleteConsumer.h>
#include "cbproject.h"
#include "projectfile.h"



#define SCI_GETTEXT 2182

using namespace clang;

inline bool IsHeaderFile(wxString name)
{
    wxString ext = name.AfterLast(_T('.')).Lower();
    if (ext.IsSameAs(FileFilters::H_EXT) ||
        ext.IsSameAs(FileFilters::HH_EXT) ||
        ext.IsSameAs(FileFilters::HPP_EXT) ||
        ext.IsSameAs(FileFilters::HXX_EXT) ||
        ext.IsSameAs(FileFilters::HPLPL_EXT) ||
        ext.IsSameAs(FileFilters::INL_EXT))
            return true;
    return false;
}
inline bool IsSourceFile(wxString name)
{
    wxString ext = name.AfterLast(_T('.')).Lower();
    if (ext.IsSameAs(FileFilters::ASM_EXT) ||
        ext.IsSameAs(FileFilters::C_EXT) ||
        ext.IsSameAs(FileFilters::CC_EXT) ||
        ext.IsSameAs(FileFilters::CPP_EXT) ||
        ext.IsSameAs(FileFilters::CXX_EXT) ||
        ext.IsSameAs(FileFilters::CPLPL_EXT))
            return true;
    return false;
}
inline bool IsCFamily(wxString name)
{
    return IsHeaderFile(name) || IsSourceFile(name);
}
inline ProjectFile* GetProjectFilePair(ProjectFile* file)
{
    cbProject* project = file->GetParentProject();
    wxString baseName = file->GetBaseName().AfterLast(wxFILE_SEP_PATH);
    FilesList& files = project->GetFilesList();
    for (auto it = files.begin(); it != files.end(); ++it)
    {
        ProjectFile* other = *it;
        if (other == file)
            continue;
        if (other->GetBaseName().AfterLast(wxFILE_SEP_PATH) == baseName)
            return other;
    }
    return nullptr;
}
// c++11 style enum to prevent name clashes.
enum class ImageKind
{
    Unknown = 0,
    NameSpace = 3,
    Class = 4,
    Struct = 7,
    Method = 10,
    Member = 13,
    Enum = 16,
    Constant = 19,
    Typedef = 22,
    Template = 25,
    Macro = 28
};
inline unsigned GetImageIndexForDeclaration(Decl* node)
{
    ImageKind kind;
    Decl::Kind declKind = node->getKind();
    switch(declKind)
    {
       case Decl::Namespace: kind = ImageKind::NameSpace;
                             break;
       case Decl::CXXRecord: kind = ImageKind::Class;
                             break;
       case Decl::ClassTemplateSpecialization:
       case Decl::ClassTemplatePartialSpecialization: kind = ImageKind::Template;
                                                      break;
       case Decl::Record:    kind = ImageKind::Struct;
                             break;
       case Decl::CXXMethod:
       case Decl::CXXConstructor:
       case Decl::CXXDestructor:
       case Decl::Function : kind = ImageKind::Method;
                             break;
       case Decl::Var:
       case Decl::Field    : kind = ImageKind::Member;
                             break;
       case Decl::Enum     : kind = ImageKind::Enum;
                             break;
       case Decl::EnumConstant: kind = ImageKind::Constant;
                                break;
       case Decl::Typedef  : kind = ImageKind::Typedef;
                             break;
       default :   kind = ImageKind::Unknown;
    }
    AccessSpecifier access = node->getAccess();
    if (access == AS_none)
        access = AS_public; //For all intents and purposes
    return static_cast<unsigned>(kind) + access;
}
inline int GetImageIndexForCompletionResult(const CodeCompletionResult& result, AccessSpecifier access)
{
    ImageKind kind;
    switch(result.CursorKind)
    {
        case CXCursor_Namespace:  kind = ImageKind::NameSpace;
                                  break;
        case CXCursor_ClassDecl:  kind = ImageKind::Class;
                                  break;
        case CXCursor_StructDecl:
             CXCursor_UnionDecl:  kind =ImageKind::Struct;
                                  break;
        case CXCursor_CXXMethod:
        case CXCursor_FunctionDecl:
        case CXCursor_Constructor:
        case CXCursor_Destructor: kind = ImageKind::Method;
                                  break;
        case CXCursor_ParmDecl:
        case CXCursor_FieldDecl:
        case CXCursor_VarDecl:    kind = ImageKind::Member;
                                  break;
        //Maybe separate those or put in constant category.
        case CXCursor_EnumConstantDecl:
        case CXCursor_EnumDecl:   kind = ImageKind::Enum;
                                  break;
        //C++11 type alias
        case CXCursor_TypeAliasDecl:
        case CXCursor_TypedefDecl:kind = ImageKind::Typedef;
                                  break;
        case CXCursor_ClassTemplate:
        case CXCursor_ClassTemplatePartialSpecialization :
                                  kind = ImageKind::Template;
                                  break;
        case CXCursor_MacroDefinition:
                                  kind = ImageKind::Macro;
                                  break;
        default : kind = ImageKind::Unknown;
    }
    if (access == AS_none)
        access = AS_public; //For all intents and purposes
    return static_cast<unsigned>(kind) + access;

}

#endif // UTIL_H_
