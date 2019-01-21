#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;


enum_4(period, jaro, leto, podzim, zima)
XTSystem::String ToStr(period &x) { return x.ToString(); }
XTSystem::String ToStr(period::enumType x) { return period(x).ToString(); }

enum_1(en1, en11)
enum_2(en2, en21, en22)
enum_3(en3, en31, en32, en33)
enum_4(en4, en41, en42, en43, en44)
enum_5(en5, en51, en52, en53, en54, en55)
enum_6(en6, en61, en62, en63, en64, en65, en66)
enum_7(en7, en71, en72, en73, en74, en75, en76, en77)
enum_8(en8, en81, en82, en83, en84, en85, en86, en87, en88)
enum_9(en9, en91, en92, en93, en94, en95, en96, en97, en98, en99)
enum_10(en10, en101, en102, en103, en104, en105, en106, en107, en108, en109, en1010)

XTSystem::String ToStr(en1 &x) { return x.ToString(); } XTSystem::String ToStr(en1::enumType x) { return en1(x).ToString(); }
XTSystem::String ToStr(en2 &x) { return x.ToString(); } XTSystem::String ToStr(en2::enumType x) { return en2(x).ToString(); }
XTSystem::String ToStr(en3 &x) { return x.ToString(); } XTSystem::String ToStr(en3::enumType x) { return en3(x).ToString(); }
XTSystem::String ToStr(en4 &x) { return x.ToString(); } XTSystem::String ToStr(en4::enumType x) { return en4(x).ToString(); }
XTSystem::String ToStr(en5 &x) { return x.ToString(); } XTSystem::String ToStr(en5::enumType x) { return en5(x).ToString(); }
XTSystem::String ToStr(en6 &x) { return x.ToString(); } XTSystem::String ToStr(en6::enumType x) { return en6(x).ToString(); }
XTSystem::String ToStr(en7 &x) { return x.ToString(); } XTSystem::String ToStr(en7::enumType x) { return en7(x).ToString(); }
XTSystem::String ToStr(en8 &x) { return x.ToString(); } XTSystem::String ToStr(en8::enumType x) { return en8(x).ToString(); }
XTSystem::String ToStr(en9 &x) { return x.ToString(); } XTSystem::String ToStr(en9::enumType x) { return en9(x).ToString(); }
XTSystem::String ToStr(en10 &x) { return x.ToString(); } XTSystem::String ToStr(en10::enumType x) { return en10(x).ToString(); }

String EnumNamesToString(const std::vector<String> &v)
{
    String s;
    for each (const String &item in v)
    {
        s += String::Format(_T("{0}"),item) + _T(",");
    }
    return s;
}

TEST_CASE(enum_test)
{
    en1 x1;
    REQUIRE(x1.numItems(),==,1)
    REQUIRE(x1,==,en1::en11)
    REQUIRE(EnumNamesToString(x1.GetNames()),==,_T("en11,"));

    en2 x2;
    REQUIRE(x2.numItems(),==,2)
    REQUIRE(x2,==,en2::en21)
    REQUIRE(EnumNamesToString(x2.GetNames()),==,_T("en21,en22,"));


    period x = period::leto;
    REQUIRE(x,==,period::leto);

    x = period::Parse(_T("jaro"));
    REQUIRE(x,==,period::jaro);

    x = period::Parse(_T("podzim"));
    REQUIRE(x,==,period::podzim);

    String s = x.ToString();
    REQUIRE(s,==,_T("podzim"));

    period y = period::podzim;

    REQUIRE(x,==,y);
    REQUIRE(x, !=, period::zima);


    period j1 = period::podzim;
    period j2 = period::jaro;
    REQUIRE(j1, >= , period::jaro)
    REQUIRE(j1, >= , j2);
    REQUIRE(period::podzim, >= , j2);

    REQUIRE(j2, <= , period::podzim)
    REQUIRE(j2, <= , j1);
    REQUIRE(period::jaro, <= , j1);

}
