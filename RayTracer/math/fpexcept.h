#pragma once

#include <float.h>

/* Based on
 https://randomascii.wordpress.com/2012/04/21/exceptional-floating-point/

 The floating-point exception flags are part of the processor state which means that they are per-thread settings. 
 Therefore, if you want exceptions enabled everywhere you need to do it in each thread, 
 typically in main/WinMain and in your thread start function, by dropping an FPExceptionEnabler object in the top of these functions.
 
 When calling out to D3D or any code that may use floating-point in a way that triggers these exceptions you need to drop in an FPExceptionDisabler object.
 
 Alternately, if most your code is not FP exception clean then it may make more sense to leave FP exceptions disabled most of the time 
 and then enable them in particular areas, such as particle systems.
*/

namespace fpexcept
{

  class enabled_scope
  {
  public:

    enabled_scope(unsigned int enable_bits = _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID)
    {
#if USE_FPEXCEPT
      _controlfp_s(&old_values, _MCW_EM, _MCW_EM);
      enable_bits &= _MCW_EM;
      _clearfp();
      _controlfp_s(0, ~enable_bits, old_values);
#endif
    }
    ~enabled_scope()
    {
#if USE_FPEXCEPT
      _controlfp_s(0, old_values, _MCW_EM);
#endif
    }

  private:
    unsigned int old_values;

    enabled_scope(const enabled_scope&);
    enabled_scope& operator=(const enabled_scope&);
  };

  class disabled_scope
  {
  public:
    disabled_scope()
    {
#if USE_FPEXCEPT
      _controlfp_s(&old_values, 0, 0);
      _controlfp_s(0, _MCW_EM, _MCW_EM);
#endif
    }
    ~disabled_scope()
    {
#if USE_FPEXCEPT
      _clearfp();
      _controlfp_s(0, old_values, _MCW_EM);
#endif
    }

  private:
    unsigned int old_values;

    disabled_scope(const disabled_scope&);
    disabled_scope& operator=(const disabled_scope&);
  };
}


