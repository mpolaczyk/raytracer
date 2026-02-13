#pragma once

#include <string>

namespace string_tools
{
    std::string to_utf8(std::wstring wstr);
    std::wstring to_utf16(std::string str);

    std::string get_name_from_godot_uri(const std::string& uri);
}
