#pragma once

#include "detri/except.hpp"

namespace detri::except
{
    DETRI_EXCEPTION_BASE(platform_exception, "Window Exception")
    DETRI_EXCEPTION(platform_exception, string_conversion_error, "String Conversion Error")
    DETRI_EXCEPTION(platform_exception, window_error, "Window Error")
}