#pragma once
#include <Windows.h>
#include <cwchar>

namespace Detached
{
    // Each DLL statically links its own MinHook lock. Serialize setup across
    // DLLs too: enabling/removing hooks suspends other process threads.
    class HookSetupGuard
    {
        HANDLE mutex=nullptr;
        bool acquired=false;
    public:
        HookSetupGuard()
        {
            wchar_t name[80]{};
            swprintf_s(name,L"Local\\FufuDetachedHookSetup-%lu",GetCurrentProcessId());
            mutex=CreateMutexW(nullptr,FALSE,name);
            if(mutex)
            {
                const auto result=WaitForSingleObject(mutex,30000);
                acquired=result==WAIT_OBJECT_0 || result==WAIT_ABANDONED;
            }
        }
        ~HookSetupGuard() { if(acquired) ReleaseMutex(mutex); if(mutex) CloseHandle(mutex); }
        explicit operator bool() const { return acquired; }
        HookSetupGuard(const HookSetupGuard&)=delete;
        HookSetupGuard& operator=(const HookSetupGuard&)=delete;
    };
}
