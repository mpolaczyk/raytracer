#include "stdafx.h"


#include "windows_minimal.h"

#include <sstream>

#include "core/string_tools.h"
#include "app/exceptions.h"

int describe_seh_exception(_EXCEPTION_POINTERS* p_exp)
{
  // https://randomascii.wordpress.com/2012/04/21/exceptional-floating-point/
  DWORD exceptionCode = p_exp->ExceptionRecord->ExceptionCode;
  const char* pDescription = NULL;
  switch (exceptionCode)
  {
  case STATUS_FLOAT_INVALID_OPERATION:
    pDescription = "float invalid operation";
    break;
  case STATUS_FLOAT_DIVIDE_BY_ZERO:
    pDescription = "float divide by zero";
    break;
  case STATUS_FLOAT_OVERFLOW:
    pDescription = "float overflow";
    break;
  case STATUS_FLOAT_UNDERFLOW:
    pDescription = "float underflow";
    break;
  case STATUS_FLOAT_INEXACT_RESULT:
    pDescription = "float inexact result";
    break;
  case STATUS_FLOAT_MULTIPLE_TRAPS:
    // This seems to occur with SSE code.
    pDescription = "float multiple traps";
    break;
  default:
    pDescription = "unknown exception";
    break;
  }

  void* pErrorOffset = 0;
#if defined(_M_IX86)
  void* pIP = (void*)p_exp->ContextRecord->Eip;
  pErrorOffset = (void*)p_exp.ContextRecord->FloatSave.ErrorOffset;
#elif defined(_M_X64)
  void* pIP = (void*)p_exp->ContextRecord->Rip;
#else
#error Unknown processor
#endif

  printf("SEH Crash with exception % x(% s) at % p!\n", exceptionCode, pDescription, pIP);

  if (pErrorOffset)
  {
    // Float exceptions may be reported in a delayed manner — report the
    // actual instruction as well.
    printf("SEH Faulting instruction may actually be at % p.\n", pErrorOffset);
  }

  // Return this value to execute the __except block and continue as if
  // all was fine, which is a terrible idea in shipping code.
  return EXCEPTION_EXECUTE_HANDLER;
  // Return this value to let the normal exception handling process
  // continue after printing diagnostics/saving crash dumps/etc.
  //return EXCEPTION_CONTINUE_SEARCH;
}

// Put this in every thread to handle SEH exception: _set_se_translator(seh_exception_handler);
// Compile with: /EHa
// https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/set-se-translator?view=msvc-170
void seh_exception_handler(unsigned int exception_code, _EXCEPTION_POINTERS* exception_info)
{
  describe_seh_exception(exception_info);  // TODO: passing p_exp to seh_exception causes invalid operation, so we can't use it in what()
  throw seh_exception(exception_code);
}

char const* seh_exception::what() const
{
  std::ostringstream oss;
  oss << "SEH exception code: " << exception_code;
  std::string str = oss.str();
  char* buff = new char[str.size() + 1];
  strcpy(buff, str.c_str());  // returning char* is fucked up
  return buff;
}

namespace
{
    struct get_hresult_description_preallocation
    {
        wchar_t* message = nullptr;
        DWORD num_chars{};
    } ghdp_data;
}
void get_hresult_description(HRESULT hr, std::ostringstream& oss)
{
    ghdp_data.num_chars = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&ghdp_data.message, 0, nullptr);
    if (ghdp_data.num_chars > 0)
    {
        oss << string_tools::to_utf8(ghdp_data.message) << "\n";
    }
    oss << std::system_category().message(hr);
}

char const* describe_hresult(HRESULT code)
{
    std::ostringstream oss;
    oss << "HR code: " << std::to_string(static_cast<uint32_t>(code)) << "\n";
    get_hresult_description(code, oss);
    std::string str = oss.str();
    char* buff = new char[str.size() + 1];
    strcpy_s(buff, str.size() + 1, str.c_str());
    return buff;
}
