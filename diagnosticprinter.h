#ifndef DIAGNOSTICPRINTER_H_
#define DIAGNOSTICPRINTER_H_

#include <string>
#include <set>


#include "util.h"
namespace clang
{
    class ASTUnit;
}
///Red squishy indicator for errors
constexpr int GetErrorIndicator()
{
    return 17;
}

///Blue squishy indicator for warnings.
constexpr int GetWarningIndicator()
{
    return 18;
}

using namespace clang;
class DiagnosticPrinter
{
public:
    DiagnosticPrinter(ASTUnit* tu);
    /// Cleans error and warning indicators
    /// from the editors
	void CleanIndicators();
	/// Marks the error and warnings emitted
	/// by the ASTUnit creation on editors.
	void MarkOnEditors();
private:
    ASTUnit* m_TU;
};


#endif // DIAGNOSTICPRINTER_H_
