#include "stdafx.h"

#include "string_tools.h"

namespace string_tools
{
    std::string to_utf8(std::wstring wstr)
    {
        const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        const std::unique_ptr<char[]> buffer(new char[bufferSize]);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.get(), bufferSize, nullptr, nullptr);
        return std::string(buffer.get());
    }

    std::wstring to_utf16(std::string str)
    {
        const int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        const std::unique_ptr<wchar_t[]> buffer(new wchar_t[bufferSize]);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.get(), bufferSize);
        return std::wstring(buffer.get());
    }
}
