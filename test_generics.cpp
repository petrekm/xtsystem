#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTSystem::Collections::Generic;

TEST_CASE(dictionary_basics)
{

    Dictionary<String, int> D;

    for (int i = 0; i < 10; i++)
    {
        int num = (i * 13) % 10;
        D.Add(String::Format(L"key_{0}", num), num);
    }
    for (int i = 0; i < 10; i++)
    {
        REQUIRE(D[String::Format(L"key_{0}", i)], == , i);
    }

    std::list<String> keys = D.Keys();
    std::list<int> values = D.Values();

    REQUIRE(keys.size(), == , 10);
    REQUIRE(values.size(), == , 10);

    //for (int i = 0; i < 10; i++)
    //{
    //    REQUIRE(String::Format(L"key_{0}", i), == , keys[i]);
    //}

}

void DoTest()
{
    dictionary_basics T;
    T.Run();
}