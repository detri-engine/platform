#include "detri/platform_exceptions.hpp"
#include "detri/platform.hpp"

namespace detri
{
    std::wstring to_wstring(const std::string& str)
    {
        if (str.empty())
        {
            return {};
        }
        std::wstring wstr;

        const int required_size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);

        if (required_size == 0)
        {
            throw except::string_conversion_error{"Couldn't convert UTF-8 to UTF-16. Windows error code: " + std::to_string(GetLastError())};
        }

        wstr.resize(required_size);

        int converted_size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], required_size);

        if (converted_size == 0)
        {
            throw except::string_conversion_error{"Couldn't convert UTF-8 to UTF-16. Windows error code: " + std::to_string(GetLastError())};
        }

        return wstr;
    }

    uint32_t processor_count() {
        const WORD group_count = GetActiveProcessorGroupCount();
        if (group_count == 0)
        {
            throw except::platform_exception{"OS reported zero processor groups."};
        }

        std::uint32_t total = 0;
        for (WORD group = 0; group < group_count; ++group)
        {
            const DWORD count = GetActiveProcessorCount(group);
            total += static_cast<std::uint32_t>(count);
        }

        if (total == 0)
        {
            throw except::platform_exception{"OS reported zero logical processors."};
        }

        return total;
    }
}
