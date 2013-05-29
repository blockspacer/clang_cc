#ifndef STRINGIO_H_
#define STRINGIO_H_

#include <wx/string.h>
#include <llvm/Support/raw_ostream.h>

/// Utility function for converting std::string to wxString
inline wxString std2wx(const std::string& str)
{
    return wxString(str.c_str(), wxConvLocal);
}
inline wxString std2wx(const char* str)
{
    return wxString(str, wxConvLocal);
}
/// Utility function for converting wxString to std::string
inline std::string wx2std(const wxString& str)
{
    return str.mb_str(wxConvLocal).data();
}

class RawwxStringOstream: public llvm::raw_ostream {
    wxString m_Str;

    /// write_impl - See raw_ostream::write_impl.
    void write_impl(const char *Ptr, size_t Size) override;

    /// current_pos - Return the current position within the stream, not
    /// counting the bytes currently in the buffer.
    uint64_t current_pos() const override { return m_Str.size(); }
public:
    ~RawwxStringOstream();

    /// str - Flushes the stream contents to the target string and returns
    ///  the string's reference.
    wxString str()
    {
        flush();
        return m_Str;
    }
    /// Flushes the stream contents and reset the target string with a new string.
    void str(const wxString& str)
    {
        flush();
        m_Str.assign(str);
    }
};
#endif // STRINGIO_H_
