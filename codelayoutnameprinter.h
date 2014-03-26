#ifndef CODELAYOUTNAMEPRINTER_H_
#define CODELAYOUTNAMEPRINTER_H_

#include <string>
#include <sstream>
#include <llvm/Support/raw_ostream.h>
#include <clang/AST/DeclVisitor.h>
#include <clang/AST/PrettyPrinter.h>
using namespace clang;
class CodeLayoutDeclarationPrinter : public ConstDeclVisitor<CodeLayoutDeclarationPrinter>
{
    llvm::raw_ostream& m_Out;
    PrintingPolicy m_Policy;

public:
    CodeLayoutDeclarationPrinter(llvm::raw_ostream& out, PrintingPolicy policy) :
        m_Out(out),
        m_Policy(policy)
    {}
    void PrintName(const NamedDecl* decl)
    {
       m_Out << decl->getNameAsString();
    }
    void PrintTemplateArguments(const TemplateArgumentList& args)
    {
        m_Out << '<';
        for (unsigned i = 0; i < args.size(); ++i)
        {
            if (i > 0)
                m_Out << ", ";
            PrintTemplateArgument(args[i]);
        }
        m_Out << '>';
    }
    void PrintTemplateArgument(const TemplateArgument& arg)
    {
       switch (arg.getKind())
       {
            case TemplateArgument::Null:
                m_Out << "null";
                break;
            case TemplateArgument::Type:
                arg.getAsType().getUnqualifiedType().print(m_Out, m_Policy);
                break;
            case TemplateArgument::Declaration:
                m_Out << arg.getAsDecl()->getNameAsString();
                break;
            case TemplateArgument::Integral:
                m_Out << arg.getAsIntegral();
                break;
            case TemplateArgument::NullPtr:
                m_Out << "nullptr";
                break;
            default:
                m_Out << "Unhandled argument kind PrintTemplateArgument";
       }
    }
    void PrintTemplateParameters(TemplateParameterList* params)
    {
        m_Out << '<';
        auto it = params->begin();
        for (;it != params->end();)
        {
            Visit(*it);
            if (++it != params->end())
                m_Out << ", ";
        }
        m_Out << '>';
    }
    void VisitNamedDecl(const NamedDecl* decl)
    {
        PrintName(decl);
    }

    void VisitTagDecl(const TagDecl* decl)
    {
        std::string name = decl->getNameAsString();
        if (name.empty())
        {
            m_Out << "<anonymous " << decl->getKindName() <<">";
        }
        else
            m_Out << name;
    }
    void VisitNamespaceDecl(const NamespaceDecl* decl)
    {
        std::string name = decl->getNameAsString();
        if (name.empty())
        {
            m_Out << "<anonymous namespace>";
        }
        else
            m_Out << name;
    }
    void VisitLinkageSpecDecl(const LinkageSpecDecl* decl)
    {
        LinkageSpecDecl::LanguageIDs lang = decl->getLanguage();
        if (lang == LinkageSpecDecl::lang_c)
            m_Out << "extern \"C\"";
        else
            m_Out << "extern \"C++\"";
    }
    void VisitVarDecl(const VarDecl* decl)
    {
        m_Out << decl->getType().getAsString(m_Policy) << ' ';
        m_Out << decl->getNameAsString();
    }
    void VisitFieldDecl(const FieldDecl* decl)
    {
        m_Out << decl->getType().getAsString(m_Policy) << ' ';
        m_Out << decl->getNameAsString();
        if (decl->isBitField())
        {
            m_Out << " : ";
            decl->getBitWidth()->printPretty(m_Out, 0, m_Policy);
        }

    }
    void VisitCXXRecordDecl(const CXXRecordDecl* decl)
    {

        std::string name = decl->getNameAsString();
        if (name.empty())
        {
            m_Out << "<anonymous " << decl->getKindName() <<">";
        }
        else
            m_Out << name;
        if (ClassTemplateDecl* declTemplate = decl->getDescribedClassTemplate())
        {
            PrintTemplateParameters(declTemplate->getTemplateParameters());
        }
    }
    void VisitClassTemplateSpecializationDecl(const ClassTemplateSpecializationDecl* decl)
    {
        m_Out << decl->getNameAsString();

        PrintTemplateArguments(decl->getTemplateArgs());

    }
    void VisitClassTemplatePartialSpecializationDecl(const ClassTemplatePartialSpecializationDecl* decl)
    {
        m_Out << decl->getNameAsString();
        PrintTemplateParameters(decl->getTemplateParameters());
        PrintTemplateArguments(decl->getTemplateArgs());
    }
    void VisitTemplateTypeParmDecl(const TemplateTypeParmDecl* decl)
    {

        if (decl->wasDeclaredWithTypename())
            m_Out << "typename ";
        else
            m_Out << "class ";
        if (decl->isParameterPack())
            m_Out << "... ";
        PrintName(decl);
        if (decl->hasDefaultArgument())
        {
            m_Out << " = " << decl->getDefaultArgument().getAsString(m_Policy);
        }
    }
    void VisitNonTypeTemplateParmDecl(const NonTypeTemplateParmDecl* decl)
    {
        m_Out << decl->getType().getAsString(m_Policy);
        if(decl->isParameterPack())
            m_Out << "...";
        m_Out << " ";
        PrintName(decl);
        if (decl->hasDefaultArgument())
        {
            m_Out << " = " ;
            decl->getDefaultArgument()->printPretty(m_Out, 0, m_Policy);
        }

    }
    void VisitTypedefDecl(const TypedefDecl* decl)
    {
        m_Out << decl->getUnderlyingType().getAsString(m_Policy)<<" ";
        PrintName(decl);
    }
    void VisitFunctionDecl(const FunctionDecl* decl)
    {
        // Return value and name
        m_Out << decl->getReturnType().getAsString(m_Policy) << ' ' << decl->getNameAsString();
        // Template paramaters if this is a Function template
        if (FunctionTemplateDecl* declTemplate = decl->getDescribedFunctionTemplate())
        {
            PrintTemplateParameters(declTemplate->getTemplateParameters());
        }

        //Function parameters if any
        m_Out << "(";
		auto it = decl->param_begin();
		for (;it != decl->param_end();)
		{
			m_Out << (*it)->getType().getAsString(m_Policy) << " " << (*it)->getNameAsString();
			if (++it != decl->param_end())
				m_Out <<", ";
		}
		m_Out << ")";
        const FunctionProtoType* ft = decl->getType()->getAs<FunctionProtoType>();
        if (ft)
        {
            if (ft->isConst())
                m_Out << " const";
            if (ft->isVolatile())
                m_Out << " volatile";
        }
        //
        if (decl->isPure())
            m_Out << " = 0";
        else if (decl->isDeletedAsWritten())
            m_Out << " = delete";
        else if (decl->isExplicitlyDefaulted())
            m_Out << " = default";
    }
};
#endif // CODELAYOUTNAMEPRINTER_H_
