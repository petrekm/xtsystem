#include "stdafx.h"
#include "XTSystem_all.h"
using namespace XTSystem;
using namespace Stack;

void ProcessInfoDemo()
{
    //ProcessInfo pi;
}


class MyStackWalker : public XTSystem::Stack::StackWalker
{
public:
    MyStackWalker() : XTSystem::Stack::StackWalker() {}
    MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : XTSystem::Stack::StackWalker(dwProcessId, hProcess) {}
    virtual void OnOutput(LPCSTR szText) { printf(szText); XTSystem::Stack::StackWalker::OnOutput(szText); }
};

//typedef MyStackWalker T_Stack;
typedef Stack::ExceptionStackWalker T_Stack;

// Test for callstack of the current thread:
void Func5() 
{ 
    
    T_Stack sw;
    sw.ShowCallstack(); 
    Console::WriteLine(sw.StackTrace());
}
void Func4() { Func5(); }
void Func3() { Func4(); }
void Func2() { Func3(); }
void TestCurrentThread() { Func2(); }

// Test for callstack of an other thread:
void TFunc5() { Sleep(10000); }
void TFunc4() { TFunc5(); }
void TFunc3() { TFunc4(); }
void TFunc2() { TFunc3(); }
DWORD WINAPI LocalThread(LPVOID) { TFunc2(); return 0; }
void TestDifferentThread() 
{
    DWORD dwThreadId;
    HANDLE hThread = CreateThread(NULL, 0, LocalThread, NULL, 0, &dwThreadId);
    Sleep(100);  // just be "sure" that the other thread is really running...
    T_Stack sw; 
    sw.ShowCallstack(hThread);
    Console::WriteLine(sw.StackTrace());
    CloseHandle(hThread);
}

/*
// Test for callstack of threads for an other process:
void TestDifferentProcess(DWORD dwProcessId)
{
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, dwProcessId);

    if (hProcess == NULL)
        return;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, dwProcessId);
    if (hSnap == INVALID_HANDLE_VALUE)
        return;

    THREADENTRY32 te;
    memset(&te, 0, sizeof(te));
    te.dwSize = sizeof(te);
    if (Thread32First(hSnap, &te) == FALSE)
    {
        CloseHandle(hSnap);
        return;
    }

    // Initialize StackWalker...
    MyStackWalker sw(dwProcessId, hProcess);
    sw.LoadModules();
    // now enum all thread for this processId
    do
    {
        if (te.th32OwnerProcessID != dwProcessId)
            continue;
        HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
        if (hThread == NULL)
            continue;
        char szTemp[100];
        //sprintf(szTemp, "\r\nThreadID: %d\r\n", te.th32ThreadID);
        sw.OnOutput(szTemp);  // output the threadId
        sw.ShowCallstack(hThread);
        CloseHandle(hThread);
    } while(Thread32Next(hSnap, &te) != FALSE);
}
*/


    // Exception handling and stack-walking example:
    LONG WINAPI ExpFilter(EXCEPTION_POINTERS* pExp, DWORD dwExpCode)
    {
        MyStackWalker sw;
        sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    void ExpTest5() { char *p = NULL; p[0] = 0; }
    void ExpTest4() { ExpTest5(); }
    void ExpTest3() { ExpTest4(); }
    void ExpTest2() { ExpTest3(); }
    void ExpTest1() { ExpTest2(); }
    void TestExceptionWalking()
    {
        __try
        {
            ExpTest1();
        }
        __except (ExpFilter(GetExceptionInformation(), GetExceptionCode()))
        {
        }
    }


void TestStack()
{
    // 1. Test with callstack for current thread
    TestCurrentThread();

    // 2. Test with different thread in the same process
    TestDifferentThread();

    // 3. Test with different process
    //TestDifferentProcess(800);  // adjust the ProcessID!

    // 4. Test of walking the callstack inside an exception
    //TestExceptionWalking();

}