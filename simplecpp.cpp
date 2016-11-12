
#include "stdafx.h"
#include <functional>

#define XTSYSTEM_EXCEPTIONS_WITH_CALLSTACK

#include "XTSystem_all.h"
#include "Testing.h"
#include "testxml.h"
#include "Logger.h"

using namespace XTSystem;
using namespace XTSystem::Collections::Generic;
using namespace Microsoft::Win32;
using namespace XTSystem::Logging;
using namespace XTSystem::Drawing;

void MakePause()
{
	Console::WriteLine(L"<press enter>");
	Console::ReadLine();
}
void GreetingDemo()
{
	Console::Write(L"Enter your name: ");
	String name = Console::ReadLine();
	for (int i=8;i<16;i++)
    {
		Console::ForeGroundColor((ConsoleColor::enumType)(i));
		Console::WriteLine(L"{0} Hello {1}! This is XTSystem. '{2}'", i, name, name.ToUpper());
	}
	Console::ForeGroundColor(ConsoleColor::Gray);
	MakePause();
}
void EnvironmentDemo()
{
	Console::WriteLine(L"Current dir: {0}",Environment::CurrentDirectory());
	Console::WriteLine(L"OsVersion: {0}",Environment::OSVersion());
	Console::WriteLine(L"Number of processors: {0}",Environment::ProcessorCount());
	Console::WriteLine(L"Machine name: {0}",Environment::MachineName());
	MakePause();
}
void MathDemo()
{
    Random R;

	Console::WriteLine(L"Math: x [deg]  |    x [rad]   |    Sin(x)    |   Cos(x)     |Random.NextDouble");
	Console::WriteLine(L"------------------------------------------------------------------------------");
	for (int x = 0; x <= 360; x += 45)
	{
		double x_rad = Math::Deg2Rad(x);
        Console::WriteLine(L"  {0:12d} | {1:12.8f} | {2:12.8f} | {3:12.8f} | {4:12.8f}", x, x_rad, Math::Cos(x_rad), Math::Sin(x_rad), R.NextDouble());
	}
	MakePause();
}

void TimerDemo()
{
	Console::Write(L"Doing something cool and measuring time ");
	XTSystem::Diagnostics::Stopwatch timer = XTSystem::Diagnostics::Stopwatch::StartNew();
    double sum = 0;
    for (int i=0;i<10000000;i++)
    {
		if (i%1000000==0) Console::Write(L".");
        sum += Math::Sin(i);
        sum += Math::Cos(i);
        sum += Math::Log(1+i);
    }
    timer.Stop();
	Console::WriteLine(L"...done!");
	Console::WriteLine(L"Time [ms]: {0}", timer.ElapsedMilliseconds());
    Console::WriteLine(L"Time [s]: {0:12.8f}", timer.ElapsedSeconds());
	MakePause();
}

void FileSystemDemo()
{
    //String rootdir = L"k:/Projects/Positioning/Troubleshooting/mapy";
	String rootdir = L"C:\\";
	Console::WriteLine(L"Show first 10 directories from '{0}'", rootdir);
	std::vector<String> files = IO::Directory::GetDirectories(rootdir);

	int i=0;
	for each (const String &file in files)
	{
		++i;
		Console::WriteLine(L"{0}", file);

		if (i>=10) break;
	}

    Console::WriteLine();
	Console::WriteLine(L"Show first 10 files from '{0}'", rootdir);
	files = IO::Directory::GetFiles(rootdir);

	i=0;
	for each (const String &file in files)
	{
		++i;
		Console::WriteLine(L"{0}", file);

		if (i>=10) break;
	}

	MakePause();
}

String VectorToString(const std::vector<String> &v)
{
    String s;
    for each (const String &item in v)
    {
        s += String::Format(L"\"{0}\"",item) + L", ";
    }
    return s;
}
void StringDemo()
{
    Console::WriteLine(L"String splitting demo:");
    String line = L"--one--three----four--five--";
    String line1 = L"one--three----four--five";

    Console::WriteLine(L"\"{0}\" split by (\"--\") \n   => {1}\n", line, VectorToString(line.Split(L"--", StringSplitOptions::None)));
    Console::WriteLine(L"\"{0}\" split by (\"--\") \n   => {1}\n", line1, VectorToString(line1.Split(L"--", StringSplitOptions::None)));

    Console::WriteLine(L"\"{0}\" split by (\"--\") (remove empty entries)\n   => {1}\n", line, VectorToString(line.Split(L"--", StringSplitOptions::RemoveEmptyEntries)));
    Console::WriteLine(L"\"{0}\" split by (\"--\") (remove empty entries)\n   => {1}\n", line1, VectorToString(line1.Split(L"--", StringSplitOptions::RemoveEmptyEntries)));

    String name = L"Hello";
    Console::WriteLine(L"\"{0}\" => ToUpper(), ToLower() => \"{1}\", \"{2}\"", name, name.ToUpper(), name.ToLower());
    
    Console::WriteLine(L"\"{0}\" => PadLeft(10,'*') => \"{1}\"", name, name.PadLeft(10,L'*'));
    Console::WriteLine(L"\"{0}\" => PadRight(10,'*') => \"{1}\"", name, name.PadRight(10,L'*'));

    Console::WriteLine();
    String text = L"asKukdfhKukakdhKukfaKuksdsKukldKukk";
    Console::WriteLine(L"\"{0}\" => Replace('Kuk','---')",text);
    Console::WriteLine(L"\"{0}\"",text.Replace(L"Kuk",L"---"));

    Console::WriteLine(L"Number format: {0}  0x{0:X}  {0:b}", 0xABCDE);

    MakePause();
}

//enum_4(Barva, cervena, modra, zelena, zluta)
enum_4_manual(Barva, cervena, 3, modra, 4, zelena, 1, zluta, 19)
//enum_5_manual(AxesMask,No,0,X,1,Y,2,Z,4,All,7)
flags_5_manual(AxesMask,No,0,X,1,Y,2,Z,4,All,7)

void EnumsDemo()
{
    Console::WriteLine(L"Enums demo:   ");
    Console::WriteLine(L"   Simple declaration: enum_4(Barva, cervena, modra, zelena, zluta)");
    Console::WriteLine(L"   Manual declaration: enum_4_manual(Barva, cervena, 3, modra, 4, zelena, 1, zluta, 19)");
    Barva b = Barva::cervena;
    Console::WriteLine(L"Barva b = Barva::cervena:   {0}",b.ToString());
    b = Barva::Parse(L"modra");
    Console::WriteLine(L"Barva b = Barva::Parse(\"modra\")  :  {0}",b.ToString());
    Console::Write(L"Barva::GetNames() : "); for each (const String &sName in Barva::GetNames()) Console::Write(L"{0}, ",sName);  Console::WriteLine();
    Console::Write(L"Barva::GetValues() : "); for each (Barva bb in Barva::GetValues()) Console::Write(L"{0}({1}), ",bb.ToString(), bb.ToInt32());  Console::WriteLine();


    Console::WriteLine();
    Console::WriteLine(L"Flags demo:   ");
    Console::WriteLine(L"   Declaration: flags_5_manual(AxesMask,No,0,X,1,Y,2,Z,4,All,7)");
    
    Console::Write(L"AxesMask::GetNames() : "); for each (const String &sName in AxesMask::GetNames()) Console::Write(L"{0}, ",sName);  Console::WriteLine();
    Console::Write(L"AxesMask::GetValues() : "); for each (AxesMask bb in AxesMask::GetValues()) Console::Write(L"{0}('{1}'), ", bb.ToInt32(),bb.ToString() );  Console::WriteLine();
    Console::WriteLine(L"AxesMask mask = AxesMask::X | AxesMask::Z");
    AxesMask mask = AxesMask::X | AxesMask::Z;
    for each (AxesMask bb in AxesMask::GetValues()) 
    {
        if (mask.ContainsFlag(bb)) Console::WriteLine(L"AxesMask {0} contains {1}", mask.ToString(), bb.ToString());
    }

    if (mask.ContainsFlag(AxesMask::X|AxesMask::Z)) Console::WriteLine(L"AxesMask {0} contains XZ", mask.ToString());

    MakePause();
}

void CollectionsDemo()
{
	List<String> lst;
	lst.Add(L"jirka");
	lst.Add(L"pepa");
	lst.Add(L"martin");
	lst.Add(L"honza");

	for each (const String &item in lst)
	{
		Console::Write(L"{0},",item);
	}

	Console::WriteLine(L"Count(): {0}",lst.Count());


	Arr<String> arr(5);
	arr[2]=L"jirka";
	arr[3] = L"pepa";

	for each (const String &item in arr)
	{
		Console::Write(L"{0},",item);
	}

	Console::WriteLine(L"Length(): {0}",arr.Length());

	//String a[] = {L"a",L"b",L"c",L"d"};
	//Arr<String> arr2(a,ARR_SIZE(a));


	Array_Init(String, arr3, {L"a",L"b",L"c",L"d"});
	Array_Init(int, arr4, {4,6,34,1,76,5});
	
	for each (const String &item in arr3)
	{
		Console::Write(L"{0},",item);
	}
	try
	{
		arr3[123] = L"asd";
	}
	catch (Exception &e)
	{
		Console::Write(L"{0},",e.Message);
	}
}

void LoggingDemo()
{
    Logging::CLogger Log;
    CFileLogger FileLog;    // default C:\\

    Log.BindImplementation(FileLog);
    Log.Info(L"==== Starting Mdlvacuum ( {0} Build: {1}, created on {2} ) ====", L"Debug64", 1234, L"2014-10-2");
    Log.Warning(L"This is warning Pi ({0}) is > E ({1}) ", Math::Pi(), Math::E());
    Log.Error(L"This is error {0} {1} {2} {3}", L"a", 123, L'A', -2.7182);
    Log.Fatal(L"This is fatal error {0} {1} {2} {3}", L"a", 123, L'A', -2.7182);

    Logging::CConsoleLogger ConsoleLog;
    Log.BindImplementation(ConsoleLog);


    Log.Info(L"==== Starting Mdlvacuum ( {0} Build: {1}, created on {2} ) ====", L"Debug64", 1234, L"2014-10-2");
    Log.Warning(L"This is warning Pi ({0}) is > E ({1}) ", Math::Pi(), Math::E());
    Log.Error(L"This is error {0} {1} {2} {3}", L"a", 123, L'A',-2.7182);
    Log.Fatal(L"This is fatal error {0} {1} {2} {3}", L"a", 123, L'A', -2.7182);

    Log.Info(LoggingCategory::dev1, L"Developer info1");
    Log.Info(LoggingCategory::dev2, L"Developer info2");


    MakePause();
}


void Test1()
{
	int a = sizeof(long long);
	RegistryKey *cfg = Registry::LocalMachine().OpenSubKey(L"SOFTWARE\\FEI\\BrickBox\\configurations",true);
	RegistryKey *rk1 = cfg->CreateSubKey(L"test2");

	String str1 = L"ahoj lidi, tohle je zapis stringu";
	rk1->SetValue(L"str1", str1);
	String str1_r = rk1->GetValue(L"str1").ToString();

	bool bool1 = true;
	rk1->SetValue(L"bool1", bool1);
	bool bool1_r = Convert::ToBoolean(rk1->GetValue(L"bool1"));

	double dbl1 = 3.141592123456789012345;
	rk1->SetValue(L"dbl1", dbl1);
	double dbl1_r = Convert::ToDouble(rk1->GetValue(L"dbl1"));

    float float1 = 2.718281F;
    rk1->SetValue(L"float1", float1);
    //float float1_r = Convert::ToSingle(rk1->GetValue(L"float1"));

	Array_Init(BYTE,binary1,{0x41,0x48,0x4f,0x4a,0x00,0x00,0x01}); 
	rk1->SetValue(L"binary1",binary1);
	//Arr<BYTE> binary1_r = (Arr<BYTE>)(rk1->GetValue(L"binary1"));

	Array_Init(String,multistring1,{ L"ahoj",L"nazdar",L"bazar"}); 
	rk1->SetValue(L"multistring1", multistring1);
	//Arr<BYTE> multistring1_r = (Arr<BYTE>)(rk1->GetValue(L"multistring1"));

	long long1 = -12345;
	rk1->SetValue(L"long1", long1);
	//long long1_r = Convert::ToInt64(rk1->GetValue(L"long1"));

	unsigned long ulong1 = 12345;
	rk1->SetValue(L"ulong1", ulong1);
	//unsigned long ulong1_r = Convert::ToUInt64(rk1->GetValue(L"ulong1"));

	int int1 = -12345;
	rk1->SetValue(L"int1", int1);
	int int1_r = Convert::ToInt32(rk1->GetValue(L"int1"));

	unsigned int uint1 = 12345;
	rk1->SetValue(L"uint1", uint1);
	//unsigned int uint1_r = Convert::ToUInt32(rk1->GetValue(L"uint1"));

	short short1 = -12;
	rk1->SetValue(L"short1", short1);
	//short short1_r = Convert::ToInt16(rk1->GetValue(L"short1"));

	unsigned short ushort1 = 12;
	rk1->SetValue(L"ushort1", ushort1);
	//unsigned short ushort1_r = Convert::ToUInt16(rk1->GetValue(L"ushort1"));

	Char char1 = L'A';
    rk1->SetValue(L"char1", char1);
    //Char char1_r = Convert::ToChar(rk1->GetValue(L"char1"));


	RegistryKey *rk2 = rk1->CreateSubKey(L"sub2");
	RegistryKey *rk3 = rk1->CreateSubKey(L"sub2\\sub3");
	RegistryKey *rk4 = rk1->CreateSubKey(L"sub4\\sub5\\sub6");

	rk1->Close();
	rk2->Close();
	rk3->Close();
	rk4->Close();

	safe_delete(rk1);
	safe_delete(rk2);
	safe_delete(rk3);
	safe_delete(rk4);
	safe_delete(cfg);
}

void RegistryDemo()
{

    RegistryKey *masterKey = Registry::LocalMachine().OpenSubKey(L"SOFTWARE\\FEI\\BrickBox\\Configurations");
    for each (const String &s in masterKey->GetSubKeyNames())
    {
        Console::WriteLine(L"{0}", s);
    }

	Console::WriteLine(L"{0}", 	masterKey->RegistryKeyPath());

    String ActiveConfiguration = masterKey->GetValue(L"ActiveConfiguration").ToString();

	RegistryKey *sub= masterKey->OpenSubKey(L"Helios450F1halonly\\Logging");
    Console::WriteLine(L"{0}", 	sub->RegistryKeyPath());

	sub->Close();
    masterKey->Close();
	
	safe_delete(sub);
	safe_delete(masterKey);

 //   String sUserData = String() + L"SOFTWARE\\FEI\BrickBox\\userdata" + L"\\" + ActiveConfiguration + L"\\" + L"System\\MdlPositioning\\MdlPositioningMgr\\";
 //   String sMachineData = String() + L"SOFTWARE\\FEI\\BrickBox\\Configurations" + L"\\" + ActiveConfiguration + L"\\" + L"bbox\\MdlPositioning\\MdlPositioningMgr\\";

 //   RegistryKey *MachineDataKey = Registry::LocalMachine().OpenSubKey(sMachineData);
 //   RegistryKey *UserDataKey = Registry::LocalMachine().OpenSubKey(sUserData);

 //   MachineDataKey->Close();
 //   UserDataKey->Close();

	//safe_delete(MachineDataKey);
	//safe_delete(UserDataKey);
}

class Ex
{
public:

    void Test5() { throw Exception(L"test exception"); }
    void Test4() { Test5(); }
    void Test3() { Test4(); }
    void Test2() { Test3(); }
    void Test1() { Test2(); }
};

void ExceptionDemo()
{
    Console::WriteLine(L"** ExceptionDemo **");
    Console::WriteLine(L" throw exception");
    Ex a;
    try
    {
        a.Test1();
    }
    catch (const Exception & e)
    {
        Console::WriteLine(L"Exception: '{0}'", e.Message);
        Console::WriteLine(L"CallStack:\n{0}", e.StackTrace);
    }

    MakePause();
}


void HResultsDemo()
{
    
    Console::WriteLine(L"** HResultsDemo **");
    HRESULT hrs[] = {
         S_OK
        ,E_ABORT
        ,E_ACCESSDENIED
        ,E_FAIL
        ,E_HANDLE
        ,E_INVALIDARG
        ,E_NOINTERFACE
        ,E_NOTIMPL
        ,E_OUTOFMEMORY
        ,E_POINTER
        ,E_UNEXPECTED
        , 0x80040810
        , 0x80040841
    };

    for (int i = 0; i < ARR_SIZE(hrs); i++)
    {
        Microsoft::HResult hr = Microsoft::HResult(hrs[i]);
        Console::WriteLine(L" hr={0} (0x{1:x}) is '{2}' description: '{3}'.", hr.ToLong(), hr.ToULong(), hr, hr.Description());
    }

    
    
}

void PrimitiveTypesDemo()
{
    Console::WriteLine(L"{0} {1}", Bool(true).Format(L"Yes/No"), Bool(false).Format(L"Yes/No"));
    Console::WriteLine(L"{0} {1}", Bool(true).Format(L"On/Off"), Bool(false).Format(L"On/Off"));
    Console::WriteLine(L"{0} {1}", Bool(true).Format(L"Ano/Ne"), Bool(false).Format(L"Ano/Ne"));

}

void ProcessDemo()
{
	try
	{

		Process p;
		p.SetCommand(L"ipconfig");
		p.Start();
	}

	catch (Exception &e)
	{
		Console::WriteLine(L"Exception: {0}", e);
	}
}

void Demo()
{
    GreetingDemo();
    EnvironmentDemo();
    MathDemo();
    TimerDemo();

    FileSystemDemo();
    StringDemo();
    CollectionsDemo();
    EnumsDemo();
    LoggingDemo();

    ExceptionDemo();
    HResultsDemo();
	ProcessDemo();
}

void TestStack();

void DoTest();

int _tmain(int argc, _TCHAR* argv[])
{

    //LoggingDemo();

    testing::TestsManager::Get().RunAllTests();
    //testing::TestsManager::Get().RunAllTestsContaining(L"io_");
    //DoTest();
    

	Demo();

    //ProcessInfoDemo();
    
    //TestStack();
    //HResultsDemo(); 
    
    MakePause();

	//Testxml T;
	//T.Test();

	return 0;

}




