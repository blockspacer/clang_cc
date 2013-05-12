#ifndef TOOLTIPEVALUATOR_H_
#define TOOLTIPEVALUATOR_H_
#include <clang/Lex/Lexer.h>
class ToolTipEvaluator : public boost::static_visitor<std::string>
{
public:
    ToolTipEvaluator(ASTUnit* tu):
        m_Tu(tu)
    {}
    std::string operator()(Decl* decl)
    {
      std:: string message = Lexer::getSourceText(CharSourceRange::getCharRange(decl->getSourceRange()),
                                                  m_Tu->getSourceManager(), m_Tu->getASTContext().getLangOpts());
      return message;
//    std::string message = named->getQualifiedNameAsString(PrintingPolicy(tu->getASTContext().getLangOpts()));
    }
    std::string operator()(Stmt* stmt)
    {
        return "";
    }
    std::string operator()(const boost::blank&)
    {
        return "";
    }
    std::string operator()(TypeLoc& tloc)
    {
        return "";
    }
    std::string operator()(const RefNode& refNode)
    {
        return "";
    }

private:
   ASTUnit* m_Tu ;
};
#endif // TOOLTIPEVALUATOR_H_
