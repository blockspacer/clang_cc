#ifndef DIAGNOSTICPRINTER_H_
#define DIAGNOSTICPRINTER_H_

#include <clang/Basic/Diagnostic.h>
#include <string>
#include <set>

#include "clangcclogger.h"
#include "util.h"
namespace clang
{
    class ASTUnit;
}
inline constexpr int GetErrorIndicator()
{
    return 17;
}
inline constexpr int GetWarningIndicator()
{
    return 18;
}
using namespace clang;
class DiagnosticPrinter
{
public:
    DiagnosticPrinter(ASTUnit* tu);
	void CleanIndicators();
	void MarkOnEditors();
private:
    ASTUnit* m_TU;
    std::set<std::string> m_Files;
};


#endif // DIAGNOSTICPRINTER_H_
