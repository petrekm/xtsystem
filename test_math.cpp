#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;

TEST_CASE(math_angledifference)
{

    REQUIRE_DOUBLE_EQ(Math::AngleDiffIn_180plus180(20,80), -60);
    REQUIRE_DOUBLE_EQ(Math::AngleDiffIn_180plus180(20,180), -160);

}

TEST_CASE(math_NormalizeAngleToRange0_360)
{
    
    for (int j=0;j<36;j++)
        for (int i=-3;i<3;i++)
    {
        double val = Math::RandomUniformAB(j*10,j*10+10);
        double modif_value = i*360 + val;
        if (Math::NormalizeAngleToRange0_360(modif_value)!=val)
        {
            REQUIRE_DOUBLE_EQ(Math::NormalizeAngleToRange0_360(modif_value), val);
        };
    }
    
}


TEST_CASE(Test_ValueInRange)
{
    REQUIRE_TRUE(Math::ValueInRange(-10, 20, 0));
    REQUIRE_TRUE(Math::ValueInRange(20, 40, 30));
    REQUIRE_TRUE(Math::ValueInRange(-10, 20, -10));
    REQUIRE_TRUE(Math::ValueInRange(-10, 20, 20));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, -15));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, 30));

    REQUIRE_FALSE(Math::ValueInRange(-10, 20, -10,RangeType::OpenClosed));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, 20,RangeType::ClosedOpen));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, -10,RangeType::OpenOpen));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, 20,RangeType::OpenOpen));
    
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, -15));
    REQUIRE_FALSE(Math::ValueInRange(-10, 20, 30));

}


TEST_CASE(Test_AngleInRange)
{
    REQUIRE_TRUE(Math::AngleInRange(-10, 20, 0));
    REQUIRE_TRUE(Math::AngleInRange(20, 40, 30));
    REQUIRE_TRUE(Math::AngleInRange(150, 210, 180));
    REQUIRE_TRUE(Math::AngleInRange(-50, -20, -30));
    REQUIRE_TRUE(Math::AngleInRange(-190, -160, 180));
    REQUIRE_FALSE(Math::AngleInRange(-190, -160, 0));

    for (int base = -180; base < 180; base+=10)
        for (int j = -2; j < 2; j++)
            for (int i = -2; i < 2; i++)
            {
                double left = 4;
                double right = 5;
                bool in1 = Math::AngleInRangeEx(j * 360 + base, left, right, i * 360 + base + right/2);
                bool out2 = !Math::AngleInRangeEx(j * 360 + base, left, right, i * 360 + base + right*2);
                bool in3 = Math::AngleInRangeEx(j * 360 + base, left, right, i * 360 + base - left/2);
                bool out4 = !Math::AngleInRangeEx(j * 360 + base, left, right, i * 360 + base - left * 2);

                if (!(in1 && out2 && in3 && out4))
                {
                    REQUIRE_TRUE(false);
                    Console::WriteLine(L"Error base, i, j = ({0}, {1}, {2})", base, i, j);
                }
            }


    REQUIRE_TRUE(Math::AngleInRange(-10, 20, 20));
    REQUIRE_TRUE(Math::AngleInRange(-10, 20, -10));

    REQUIRE_TRUE(Math::AngleInRange(10, 20, 20+123*360));
    REQUIRE_TRUE(Math::AngleInRange(10, 20, 10-321*360));

}

