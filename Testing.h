#pragma once
#include "XTSystem_all.h"
#include <list>

namespace testing
{

#define STR(A) _T(#A)

#ifdef _MSC_VER

#if (_MSC_VER >= 1600)

#define REQUIRE(A, OP, B) { \
    auto _A = (A); auto _B=(B); \
    nChecks++; \
    bool asdfqwer_success = false; \
    if (_A OP _B) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(_A); \
    XTSystem::String sValB = TestsManager::ToString(_B); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" ") + XTSystem::String(STR(OP)) + XTSystem::String(L" ") + XTSystem::String(STR(B)); \
    XTSystem::String sOper = XTSystem::String(STR(OP)); \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, STR(B), sValB, sOper); \
    } \
}

#define REQUIRE_DOUBLE_EQ(A, B) { \
    auto _A = (A); auto _B=(B); nChecks++; \
    bool asdfqwer_success = false; \
    if (fabs(_A - _B) <= DBL_EPSILON) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(_A); \
    XTSystem::String sValB = TestsManager::ToString(_B); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" ") + XTSystem::String(_T("===")) + XTSystem::String(L" ") + XTSystem::String(STR(B)); \
    XTSystem::String sOper = XTSystem::String(_T("===")); \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, STR(B), sValB, sOper); \
    } \
}

#define REQUIRE_TRUE(A) { \
    auto _A = (A); nChecks++; \
    bool asdfqwer_success = false; \
    if (_A) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(_A); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" == true") ; \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, L"", L"", L""); \
    } \
}


#define REQUIRE_FALSE(A) { \
    auto _A = (A); nChecks++; \
    bool asdfqwer_success = false; \
    if (!_A) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(_A); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" == false") ; \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, L"", L"", L""); \
    } \
}



#else


#define REQUIRE(A, OP, B) { \
    nChecks++; \
    bool asdfqwer_success = false; \
    if ((A) OP (B)) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(L"..."); \
    XTSystem::String sValB = TestsManager::ToString(L"..."); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" ") + XTSystem::String(STR(OP)) + XTSystem::String(L" ") + XTSystem::String(STR(B)); \
    XTSystem::String sOper = XTSystem::String(STR(OP)); \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, STR(B), sValB, sOper); \
    } \
}

#define REQUIRE_DOUBLE_EQ(A, B) { \
    nChecks++; \
    bool asdfqwer_success = false; \
    if (fabs((A) - (B)) <= DBL_EPSILON) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(L"..."); \
    XTSystem::String sValB = TestsManager::ToString(L"..."); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" ") + XTSystem::String(_T("===")) + XTSystem::String(L" ") + XTSystem::String(STR(B)); \
    XTSystem::String sOper = XTSystem::String(_T("===")); \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, STR(B), sValB, sOper); \
    } \
}

#define REQUIRE_TRUE(A) { \
    nChecks++; \
    bool asdfqwer_success = false; \
    if ((A)) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(L"..."); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" == true") ; \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, L"", L"", L""); \
    } \
}
#define REQUIRE_FALSE(A) { \
    nChecks++; \
    bool asdfqwer_success = false; \
    if (!(A)) { \
    asdfqwer_success = true; \
    } else { \
    asdfqwer_success = false;  nFailedChecks++; \
    } \
    if (asdfqwer_success==false || TestsManager::Get().ShowAllTests()) { \
    XTSystem::String sValA = TestsManager::ToString(L"..."); \
    XTSystem::String sExpr = XTSystem::String(STR(A)) + XTSystem::String(L" == false") ; \
    TestsManager::MakeInfo(asdfqwer_success, sExpr, STR(A), sValA, L"", L"", L""); \
    } \
}


#endif
#endif

#define EXPECT_THAT REQUIRE

class Test
{

public:
    Test() :nChecks(0),nFailedChecks(0) {}
    virtual ~Test() {}
    void Run()
    {
        XTSystem::Console::WriteLine(L"[+]TEST '{0}' started", GetName());
        try 
        {
            RunTest();

        } 
        catch (...)
        {
            XTSystem::Console::WriteLine(L"Test failed with unknown exception");
            nChecks++;
            nFailedChecks++;
        }
        //XTSystem::Console::WriteLine(L"[-]TEST '{0}' finished", GetName());
        XTSystem::Console::WriteLine();
        XTSystem::Console::WriteLine();
    }

    int GetNumberOfChecks() const { return nChecks;}
    int GetNumberOfFailedChecks() const { return nFailedChecks; }
	virtual XTSystem::String GetName() { return L""; };
protected:
    virtual void RunTest() {};


    int nChecks;
    int nFailedChecks;
};


typedef Test*T_TestCreatingFunction(void);

class TestsManager
{
public:
    bool ShowAllTests() { return showall_tests; }
    static TestsManager &Get()
    {
        static TestsManager instance;
        return instance;
    }
    void Add(T_TestCreatingFunction *cf) { alltests_factory.push_back(cf); }
    void RunAllTests()
    {

        int nTests=0;
        int nFailed=0;

        int nTotalChecks=0;
        int nTotalFailedChecks=0;

        std::vector<testing::Test*> alltests;
        for (int i=0;i<alltests_factory.size(); i++)
        {
            T_TestCreatingFunction* func = alltests_factory[i];
            alltests.push_back((*func)());
        }


        for (int i=0;i<alltests.size(); i++)
        {
            testing::Test *t = alltests[i];
            t->Run();
            int nChecks_inSingleTest = t->GetNumberOfChecks();
            int nFailedChecks_inSingleTest = t->GetNumberOfFailedChecks();

            nTotalChecks += nChecks_inSingleTest;
            nTotalFailedChecks += nFailedChecks_inSingleTest;

            if (nFailedChecks_inSingleTest>0) nFailed++;
            nTests++;
        }

        if (nFailed>0)
        {
            XTSystem::Console::WriteLine(L"!Tests failed! (failed : {0}/ total: {1})  (failed checks : {2}/ total checks: {3})", nFailed, nTests, nTotalFailedChecks, nTotalChecks);
        } else
        {
            XTSystem::Console::WriteLine(L"All tests succeed. (failed: {0}/ total: {1})  (failed checks : {2}/ total checks: {3})", nFailed, nTests, nTotalFailedChecks, nTotalChecks);
        }

        for (int i=0;i<alltests.size(); i++) delete alltests[i];

    }

	void RunAllTestsContaining(const XTSystem::String &tag)
	{

		int nTests = 0;
		int nFailed = 0;

		int nTotalChecks = 0;
		int nTotalFailedChecks = 0;

		std::vector<testing::Test*> alltests;
		for (int i = 0; i<alltests_factory.size(); i++)
		{
			T_TestCreatingFunction* func = alltests_factory[i];
			alltests.push_back((*func)());
		}


		for (int i = 0; i<alltests.size(); i++)
		{
			testing::Test *t = alltests[i];
			if (!t->GetName().Contains(tag)) continue;

			t->Run();
			int nChecks_inSingleTest = t->GetNumberOfChecks();
			int nFailedChecks_inSingleTest = t->GetNumberOfFailedChecks();

			nTotalChecks += nChecks_inSingleTest;
			nTotalFailedChecks += nFailedChecks_inSingleTest;

			if (nFailedChecks_inSingleTest>0) nFailed++;
			nTests++;
		}

		if (nFailed>0)
		{
			XTSystem::Console::WriteLine(L"!Tests failed! (failed : {0}/ total: {1})  (failed checks : {2}/ total checks: {3})", nFailed, nTests, nTotalFailedChecks, nTotalChecks);
		}
		else
		{
			XTSystem::Console::WriteLine(L"All tests succeed. (failed: {0}/ total: {1})  (failed checks : {2}/ total checks: {3})", nFailed, nTests, nTotalFailedChecks, nTotalChecks);
		}

		for (int i = 0; i<alltests.size(); i++) delete alltests[i];

	}

    template<class T> static XTSystem::String ToString( T &t ) {return ToStr(t); } //{ return t.ToString(); }
    //template<class T> static XTSystem::String ToString( const T &t ) {return t.ToString(); } //{ return t.ToString(); }
    static XTSystem::String TestsManager::ToString( double x ) { return XTSystem::String::Format(L"{0}",x); }
    static XTSystem::String TestsManager::ToString( int x ) { return XTSystem::String::Format(L"{0}",x);}
    static XTSystem::String TestsManager::ToString( long x ) { return XTSystem::String::Format(L"{0}",x); }
    static XTSystem::String TestsManager::ToString( float x ) { return XTSystem::String::Format(L"{0}",x); }
    static XTSystem::String TestsManager::ToString( const XTSystem::String &x ) { return XTSystem::String(x); }
    static XTSystem::String TestsManager::ToString( XTSystem::String &x ) { return XTSystem::String(x); }

    static XTSystem::String TestsManager::ToString( wchar_t x ) { return XTSystem::String::Format(L"{0}",x); }
    static XTSystem::String TestsManager::ToString( const wchar_t* x ) { return XTSystem::String(x); }
    //static XTSystem::String TestsManager::ToString(std::wstring x) { return XTSystem::String::FromWstring(x); }
    static XTSystem::String TestsManager::ToString(size_t x) { return XTSystem::String::Format(L"{0}",x);}
    static XTSystem::String TestsManager::ToString(bool x) { return x?L"True":L"False";}


    static void MakeInfo(bool wasSuccessful, const XTSystem::String &sExpr, const XTSystem::String &sA, const XTSystem::String &sValA, 
        const XTSystem::String &sB, const XTSystem::String &sValB, const XTSystem::String &sOper)
    {

        if (wasSuccessful)
        {
            XTSystem::ConsoleColor fgc = XTSystem::Console::ForeGroundColor();
            XTSystem::ConsoleColor bgc = XTSystem::Console::BackGroundColor();

            XTSystem::Console::ForeGroundColor(XTSystem::ConsoleColor::Green);
            XTSystem::Console::BackGroundColor(XTSystem::ConsoleColor::Black);
            XTSystem::Console::WriteLine(L"pass: ({0})             ({1} {2} {3})  ",sExpr, sValA, sOper, sValB);
            XTSystem::Console::ForeGroundColor(fgc);
            XTSystem::Console::BackGroundColor(bgc);
        }
        else
        {
            XTSystem::ConsoleColor fgc = XTSystem::Console::ForeGroundColor();
            XTSystem::ConsoleColor bgc = XTSystem::Console::BackGroundColor();

            XTSystem::Console::ForeGroundColor(XTSystem::ConsoleColor::Red);
            XTSystem::Console::BackGroundColor(XTSystem::ConsoleColor::Black);
            XTSystem::Console::WriteLine(L"FAIL! ({0})             ({1} {2} {3})  ",sExpr, sValA, sOper, sValB);
            XTSystem::Console::ForeGroundColor(fgc);
            XTSystem::Console::BackGroundColor(bgc);

        }
    }
private:
    TestsManager()
    {
        showall_tests = true;
    }
    bool showall_tests;
    std::vector<T_TestCreatingFunction*>  alltests_factory;
};


#define TEST_CASE(A) \
class A:public Test {\
  public: virtual String GetName() { return _T(#A); } \
  protected: virtual void RunTest(); \
  public: static Test* Create() { return new A(); } \
public: static bool InitializeTest() { testing::TestsManager::Get().Add(&A::Create); return true; } \
}; \
namespace A##_ns{ bool b##A##_b = A::InitializeTest(); }\
void A##::RunTest()




}