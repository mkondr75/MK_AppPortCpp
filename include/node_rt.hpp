#pragma once

bool WaitForPort(int port, int timeoutMs);
void StartNode(Config &cfg);
void StopNode(void);
DWORD GetNodePID();