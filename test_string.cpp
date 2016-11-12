#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
TEST_CASE(string_format)
{
    String s = String::Format(L"Formated string: {0} {1} {2} {0}", 12, 3.14, L'X');
    REQUIRE(s,==,L"Formated string: 12 3.14 X 12");
}


TEST_CASE(string_upperlower)
{
    String kuku   = L"ahOj$#@pePo$#@juro$#@";
    String KUKU = kuku.ToUpper();
    REQUIRE(KUKU,==,L"AHOJ$#@PEPO$#@JURO$#@");
    String kuku1 = KUKU.ToLower();
    REQUIRE(kuku1,==,L"ahoj$#@pepo$#@juro$#@");
}

TEST_CASE(string_substring)
{

    String s = L"alpha, beta, gamma";
    String ss = s.Substring(2,3);
    REQUIRE(ss,==,L"pha");
}

TEST_CASE(string_IndexOf_LastIndexOf)
{
    String s = L"alpha, beta, gamma";
    REQUIRE(s.IndexOf(L"beta"), ==, 7);
    REQUIRE(s.LastIndexOf(L"beta"), ==, 7);

    REQUIRE(s.IndexOf(L","), ==, 5);
    REQUIRE(s.LastIndexOf(L","), ==, 11);
}

TEST_CASE(string_ToCharArray)
{
    String s = L"alpha, beta, gamma";
    std::vector<Char> arr = s.ToCharArray();
    REQUIRE(arr.size(), == , 18);
    for (int i=0; i<18; i++)
    {
        REQUIRE(arr[i],==,s[i]);
    }
}

TEST_CASE(string_replace)
{
    REQUIRE(String(L"axbxcxdxexfxgx").Replace(L'x',L'-'), == , L"a-b-c-d-e-f-g-");
    String s = L"axybxycxydxyexyfxygxyasd";
    String rep=L"<-->";
    String s1 = s.Replace(L"xy",rep);
    REQUIRE(s1, ==, L"a<-->b<-->c<-->d<-->e<-->f<-->g<-->asd");
}

TEST_CASE(string_remove)
{
    String s = L"alpha, beta, gamma";
    s = s.Remove(5,6);
    REQUIRE(s, == , L"alpha, gamma");
}

TEST_CASE(string_insert)
{
    String s = L"alpha, gamma, delta";
    s = s.Insert(7, L"beta, ");
    REQUIRE(s, == , L"alpha, beta, gamma, delta");
}

TEST_CASE(string_startwith_endwith)
{
    String s = L"alpha, gamma, delta";
    REQUIRE(s.StartsWith(L"alph"), == , true);
    REQUIRE(s.EndsWith(L"lta"), == , true);
    REQUIRE(s.Contains(L"gamma"), == , true);
    REQUIRE(s.Contains(L"Del"), == , false);

}

TEST_CASE(string_padding)
{
    String s;
    s = L"1234567";
    s = s.PadLeft(10,L'X');
    REQUIRE(s, == , L"XXX1234567");
    s = s.PadRight(15,L'Y');
    REQUIRE(s, == , L"XXX1234567YYYYY");

    const String &cs = s;
    String s1 = cs.PadLeft(20,L'Q');
}

TEST_CASE(string_compare)
{
    REQUIRE(String::Compare(L"a",L"A"), == , -1);
    REQUIRE(String::Compare(L"a",L"A",true), == , 0);
    REQUIRE(String::Compare(L"ab",L"A",true), == , 1);
    REQUIRE(String::Compare(L"ab",L"Ab"), == , -1);
    REQUIRE(String::Compare(L"ab",L"AB"), == , -1);


}

TEST_CASE(string_split)
{
    String s= L"ahoj$#@pepo$#@juro$#@$#@matito$#@";
    std::vector<String> v = s.Split(L"$#@",StringSplitOptions::None);
    REQUIRE(v.size(),==,6);
    v = s.Split(L"$#@",StringSplitOptions::RemoveEmptyEntries);
    REQUIRE(v.size(),==,4);
    s = L" element kuk 123 ";
    v = s.Split(L" ",StringSplitOptions::RemoveEmptyEntries);
    REQUIRE(v.size(),==,3);
    s = L"element kuk 123 ";
    v = s.Split(L" ",StringSplitOptions::RemoveEmptyEntries);
    REQUIRE(v.size(),==,3);
    s = L"element kuk 123";
    v = s.Split(L" ",StringSplitOptions::RemoveEmptyEntries);
    REQUIRE(v.size(),==,3);

}

TEST_CASE(string_Trim)
{
    String s = L"  \tahoj  \n\v\t";
    REQUIRE(s.Trim(),==,L"ahoj");
    REQUIRE(s.TrimStart(L"\n\t\v "),==,L"ahoj  \n\v\t");
    REQUIRE(s.TrimEnd(L"\n\t\v "),==,L"  \tahoj");
    REQUIRE(String(L"\r\n").Trim(), == , L"");

    REQUIRE(String(L"\r\nkuk").TrimStart(L"\r\n"), == , L"kuk");
    REQUIRE(String(L"\r\nkuk").TrimStart(L"\r"), == , L"\nkuk");
    REQUIRE(String(L"\r  \t\t\n").TrimStart(L"\r\n\t\v "), == , L"");

    REQUIRE(String(L"aa\r\n").TrimEnd(L"\r\n"), == , L"aa");
    REQUIRE(String(L"aa\r\n").TrimEnd(L"\n"), == , L"aa\r");
    REQUIRE(String(L"  \t\r\n").TrimEnd(L"\n\r \t"), == , L"");
}


class OOO
{
public:
    virtual String ToString() const
    {
        return L"[OOO]";
    }
};
class MyClass : public OOO
{
public:
    virtual String ToString() const
    {
        return L"[MyClass]";
    }
};

enum_3(letters, letterA, letterB, letterC);

TEST_CASE(string_OverTyping)
{
    
    OOO O;
    MyClass M;

    String s = String::Format(L"{0}", O);
    REQUIRE(s, == , L"[OOO]");


    s = String::Format(L"{0}", M);
    REQUIRE(s, == , L"[MyClass]");

    OOO &X = M;  // hide the type
    s = String::Format(L"{0}", X);
    REQUIRE(s, == , L"[MyClass]");


    letters letter;

    s = String::Format(L"{0}", letter);
    REQUIRE(s, == , L"letterA");
}