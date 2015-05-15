#pragma once

#include <manager.h>


class Compiler;
class cbProject;
class CompilerCommandGenerator;
class ProjectBuildTarget;
class ProjectFile;
class CommandLineGenerator
{
public:
    CommandLineGenerator(Compiler* compiler, cbProject* project);
    wxArrayString GetCompileFileCommand(ProjectBuildTarget* target, ProjectFile* pf) const;
    void AddCommandsToArray(const wxString& cmds, wxArrayString& array, bool isWaitCmd = false, bool isLinkCmd = false) const;
private:
    Compiler* m_Compiler;
    cbProject* m_Project;
    CompilerCommandGenerator* m_Generator;

};
