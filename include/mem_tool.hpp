#include <windows.h>

void PrintMemoryBreakdown();
void TrimMemory(HANDLE hProcess);
void TrimTree(DWORD parentId);