#pragma once

#include <string>

#ifdef _WIN32
#ifndef DETRI_WINDOWS_INCLUDES
#define DETRI_WINDOWS_INCLUDES
#include <windows.h>
#include <windowsx.h>
#endif
#endif

namespace detri
{
    std::wstring to_wstring(const std::string& str);

    uint32_t processor_count();
}
