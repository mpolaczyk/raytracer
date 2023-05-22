rem https://devblogs.microsoft.com/cppblog/introducing-c-build-insights/
rem https://learn.microsoft.com/pl-pl/windows-hardware/test/wpt/windows-performance-analyzer

vs2022_cmd_prompt_x64.bat
vcperf /start RayTracerBuildInsightsSession
pause