#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTSystem::Diagnostics;

TEST_CASE(diagnostics_normalstart)
{

    Stopwatch W;
    W.Start();
    Threading::Thread::Sleep(100);
    W.Stop();
    Threading::Thread::Sleep(100);
    W.Start();	// no reset
    Threading::Thread::Sleep(100);
    W.Stop();

    REQUIRE(Math::Abs(W.ElapsedSeconds() - 0.200), <= , 0.001);
}

TEST_CASE(diagnostics_reset)
{

    Stopwatch W;

    W.Start();
    Threading::Thread::Sleep(100);
    W.Stop();
    Threading::Thread::Sleep(100);
    W.Restart();	// no reset
    Threading::Thread::Sleep(100);
    W.Stop();

    REQUIRE(Math::Abs(W.ElapsedSeconds() - 0.100), <= , 0.001);

}