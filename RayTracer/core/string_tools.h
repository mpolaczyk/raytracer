#pragma once

#include <string>

namespace string_tools
{
    std::string to_utf8(std::wstring wstr);
    std::wstring to_utf16(std::string str);
}
