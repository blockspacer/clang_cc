#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <wx/string.h>
#include <boost/foreach.hpp>
#include "filefilters.h"
#include <clang/AST/Decl.h>
#include <clang/Sema/CodeCompleteConsumer.h>
#include "cbproject.h"
#include "projectfile.h"



#define SCI_GETTEXT 2182

using namespace clang;
inline wxString std2wx(const std::string& s)
{
     wxString wx;
     const char* my_string=s.c_str();
     wxMBConvUTF8 *wxconv= new wxMBConvUTF8();
     wx=wxString(wxconv->cMB2WC(my_string),wxConvUTF8);
     delete wxconv;
     // test if conversion works of not. In case it fails convert from Ascii
     if (wx.length()==0)
       wx=wxString(wxString::FromAscii(s.c_str()));
     return wx;
}
inline std::string wx2std(const wxString& s)
{
     std::string s2;
     if (s.IsAscii())
     {
       s2=s.ToAscii();
     } else
     {
       const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(s);
       const char *tmp_str = (const char*) tmp_buf;
       s2=std::string(tmp_str, strlen(tmp_str));
     }
     return s2;
}

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
enum ImageKind
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
inline int GetImageIndexForDeclaration(Decl* node)
{
    ImageKind kind;
    Decl::Kind declKind = node->getKind();
    switch(declKind)
    {
       case Decl::Namespace: kind = NameSpace;
                             break;
       case Decl::CXXRecord: kind = Class;
                             break;
       case Decl::ClassTemplatePartialSpecialization: kind = Template;
                                                      break;
       case Decl::Record:    kind = Struct;
                             break;
       case Decl::CXXMethod:
       case Decl::CXXConstructor:
       case Decl::CXXDestructor:
       case Decl::Function : kind = Method;
                             break;
       case Decl::Var:
       case Decl::Field    : kind = Member;
                             break;
       case Decl::Enum     : kind = Enum;
                             break;
       case Decl::EnumConstant: kind = Constant;
                                break;
       case Decl::Typedef  : kind = Typedef;
                             break;
       default :   kind = Unknown;
    }
    AccessSpecifier access = node->getAccess();
    if (access == AS_none)
        access = AS_public; //For all intents and purposes
    return kind + access;
}
inline int GetImageIndexForCompletionResult(const CodeCompletionResult& result, AccessSpecifier access)
{
    ImageKind kind;
    switch(result.CursorKind)
    {
        case CXCursor_Namespace:  kind = NameSpace;
                                  break;
        case CXCursor_ClassDecl:  kind = Class;
                                  break;
        case CXCursor_StructDecl:
             CXCursor_UnionDecl:  kind = Struct;
                                  break;
        case CXCursor_CXXMethod:
        case CXCursor_FunctionDecl:
        case CXCursor_Constructor:
        case CXCursor_Destructor: kind = Method;
                                  break;
        case CXCursor_ParmDecl:
        case CXCursor_FieldDecl:
        case CXCursor_VarDecl:    kind = Member;
                                  break;
        //Maybe separate those or put in constant category.
        case CXCursor_EnumConstantDecl:
        case CXCursor_EnumDecl:   kind = Enum;
                                  break;
        //C++11 type alias
        case CXCursor_TypeAliasDecl:
        case CXCursor_TypedefDecl:kind = Typedef;
                                  break;
        case CXCursor_ClassTemplate:
        case CXCursor_ClassTemplatePartialSpecialization :
                                  kind = Template;
                                  break;
        case CXCursor_MacroDefinition:
                                  kind = Macro;
                                  break;
        default : kind = Unknown;
    }
    if (access == AS_none)
        access = AS_public; //For all intents and purposes
    return kind + access;

}

#endif // UTIL_H_
