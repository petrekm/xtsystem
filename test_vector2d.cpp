#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"
#include "XTMath3d_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTMath3d;

TEST_CASE(vector2_construction)
{
    Vector2 v;

    REQUIRE_DOUBLE_EQ(v.x, 0);
    REQUIRE_DOUBLE_EQ(v.y, 0);

    Vector2 v1(10,20);
    REQUIRE_DOUBLE_EQ(v1.x, 10);
    REQUIRE_DOUBLE_EQ(v1.y, 20);

    v = v1;
    REQUIRE_DOUBLE_EQ(v.x, v1.x);
    REQUIRE_DOUBLE_EQ(v.y, v1.y);

}
