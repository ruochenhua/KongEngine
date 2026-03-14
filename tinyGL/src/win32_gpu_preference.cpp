// 让 Windows 双显卡笔记本优先使用独显（NVIDIA / AMD）。
// 驱动在进程启动时检查 EXE 是否导出以下符号，仅 EXE 导出有效，DLL 导出无效。
#if defined(_WIN32) || defined(_MSC_VER)

#ifdef __cplusplus
extern "C" {
#endif

// NVIDIA Optimus：EXE 导出此符号且值为 1 时，使用高性能 GPU
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;

// AMD PowerXpress：EXE 导出此符号且值为 1 时，使用高性能 GPU
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

#endif
