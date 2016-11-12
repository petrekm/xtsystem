#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;

TEST_CASE(random_test)
{
    Random R(10);
    for (int i = 0; i<100; i++)
    {
        int r = R.Next();
        double d = R.NextDouble();
        if (r<0 || r>Int32::MaxValue())
        {
            REQUIRE(r, >= , 0);
            REQUIRE(r, <= , Int32::MaxValue());
        }

        if (d<0 || d> 1.0)
        {
            REQUIRE(d, >= , 0.0);
            REQUIRE(d, <= , 1.0);
        }

    }
}