
#include "commandlinegenerator.h"
#include "clangcclogger.h"
#include <compiler.h>
#include <cbproject.h>
#include <compilercommandgenerator.h>
#include <compilerfactory.h>
#include <wx/arrstr.h>
#include <wx/hashmap.h>

#define COMPILER_SIMPLE_LOG     "SLOG:"
#define COMPILER_NOTE_LOG       "SLOG:NLOG:"
#define COMPILER_WARNING_LOG    "SLOG:WLOG:"
#define COMPILER_TARGET_CHANGE  "TGT:"
#define COMPILER_WAIT           "WAIT"
#define COMPILER_WAIT_LINK      "LINK"

CommandLineGenerator::CommandLineGenerator(Compiler* compiler,
                                           cbProject* project):
    m_Compiler(compiler),
    m_Project(project)
{
    m_Generator = m_Compiler->GetCommandGenerator(m_Project);
}
wxArrayString CommandLineGenerator::GetCompileFileCommand(ProjectBuildTarget* target, ProjectFile* pf) const
{
    wxArrayString ret;
    wxArrayString ret_generated;

    // is it compilable?
    if (!pf || !pf->compile)
        return ret;

    if (pf->compilerVar.IsEmpty())
    {
        LoggerAccess::Get()->Log("Cannot resolve compiler var for project file.");
        return ret;
    }

    Compiler* compiler = target
                       ? CompilerFactory::GetCompiler(target->GetCompilerID())
                       : m_Compiler;
    if (!compiler)
    {
        LoggerAccess::Get()->Log("Can't access compiler for file.");
        return ret;
    }

    const pfDetails& pfd = pf->GetFileDetails(target);
    wxString object      = (compiler->GetSwitches().UseFlatObjects)
                         ? pfd.object_file_flat : pfd.object_file;
    wxString object_dir  = (compiler->GetSwitches().UseFlatObjects)
                         ? pfd.object_dir_flat_native : pfd.object_dir_native;
    // lookup file's type
    const FileType ft = FileTypeOf(pf->relativeFilename);

    bool is_resource = ft == ftResource;
    bool is_header   = ft == ftHeader;

    // allowed resources under all platforms: makes sense when cross-compiling for
    // windows under linux.
    // and anyway, if the user is dumb enough to try to compile resources without
    // having a resource compiler, (s)he deserves the upcoming build error ;)

    wxString compiler_cmd;
    if (!is_header || compiler->GetSwitches().supportsPCH)
    {
        const CompilerTool* tool = compiler->GetCompilerTool(is_resource ? ctCompileResourceCmd : ctCompileObjectCmd, pf->file.GetExt());

        // does it generate other files to compile?
        for (size_t i = 0; i < pf->generatedFiles.size(); ++i)
            AppendArray(GetCompileFileCommand(target, pf->generatedFiles[i]), ret_generated); // recurse

        pfCustomBuild& pcfb = pf->customBuild[compiler->GetID()];
        if (pcfb.useCustomBuildCommand)
            compiler_cmd = pcfb.buildCommand;
        else if (tool)
            compiler_cmd = tool->command;
        else
            compiler_cmd = wxEmptyString;

        wxString source_file;
        if (compiler->GetSwitches().UseFullSourcePaths)
            source_file = UnixFilename(pfd.source_file_absolute_native);
        else
            source_file = pfd.source_file;

        // for resource files, use short from if path because if windres bug with spaces-in-paths
        if (is_resource && compiler->GetSwitches().UseFullSourcePaths)
            source_file = pf->file.GetShortPath();

        QuoteStringIfNeeded(source_file);

        m_Generator->GenerateCommandLine(compiler_cmd,
                                          target,
                                          pf,
                                          source_file,
                                          object,
                                          pfd.object_file_flat,
                                          pfd.dep_file);
    }

    if (!is_header && compiler_cmd.IsEmpty())
    {
        ret.Add(  wxString(COMPILER_SIMPLE_LOG)
                + "Skipping file (no compiler program set): "
                + pfd.source_file_native );
        return ret;
    }

    AddCommandsToArray(compiler_cmd, ret);

    if (is_header)
        ret.Add(wxString(COMPILER_WAIT));

    if (ret_generated.GetCount())
    {
        // not only append commands for (any) generated files to be compiled
        // but also insert a "pause" to allow this file to generate its files first
        if (!is_header) // if is_header, the "pause" has already been added
            ret.Add(wxString(COMPILER_WAIT));
        AppendArray(ret_generated, ret);
    }

    // if it's a PCH, delete the previously generated PCH to avoid problems
    // (it 'll be recreated anyway)
    if ( (ft == ftHeader) && pf->compile )
    {
        wxString object_abs = (compiler->GetSwitches().UseFlatObjects)
                            ? pfd.object_file_flat_absolute_native
                            : pfd.object_file_absolute_native;

        if ( !wxRemoveFile(object_abs) )
            LoggerAccess::Get()->Log("Cannot remove old PCH file:\n" + object_abs);
    }

    return ret;
}
void CommandLineGenerator::AddCommandsToArray(const wxString& cmds, wxArrayString& array, bool isWaitCmd, bool isLinkCmd) const
{
    wxString cmd = cmds;
    while (!cmd.IsEmpty())
    {
        int idx = cmd.Find("\n");
        wxString cmdpart = idx != -1 ? cmd.Left(idx) : cmd;
        cmdpart.Trim(false);
        cmdpart.Trim(true);
        if (!cmdpart.IsEmpty())
        {
            if (isWaitCmd)
                array.Add(wxString(COMPILER_WAIT));
            if (isLinkCmd)
                array.Add(wxString(COMPILER_WAIT_LINK));
            array.Add(cmdpart);
        }
        if (idx == -1)
            break;
        cmd.Remove(0, idx + 1);
    }
}
