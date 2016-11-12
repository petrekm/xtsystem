#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace Microsoft;

TEST_CASE(hresults_string)
{
    REQUIRE(HResult(S_OK).ToString(), == , L"S_OK");
    REQUIRE(HResult(E_FAIL).ToString(), == , L"E_FAIL");
}
