#include "stringio.h"

RawwxStringOstream::~RawwxStringOstream()
{
    flush();
}
void RawwxStringOstream::write_impl(const char *Ptr, size_t Size)
{
#if !wxCHECK_VERSION(2, 9, 5)
    m_Str << std2wx(std::string(Ptr,Size));
#else
    m_Str.Append(Ptr, Size);
#endif
}
