#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTSystem::IO;


TEST_CASE(io_directory_current)
{
	
	String curr = Directory::GetCurrentDirectory_();


	for (const String &dir : Directory::GetDirectories(curr))
	{
		Console::WriteLine(dir);
	}

	bool exist = Directory::Exists(curr);
	String dirNew = Path::Combine(curr, L"newdir1");
	
	
	bool e1 = Directory::Exists(dirNew);
	REQUIRE(e1, == , false);
	Console::WriteLine(L"---");


	Directory::CreateDirectory_(dirNew);

	for (const String &dir : Directory::GetDirectories(curr))
	{
		Console::WriteLine(dir);
	}
	e1 = Directory::Exists(dirNew);
	REQUIRE(e1, == , true);
	Console::WriteLine(L"---");

	Directory::Delete(dirNew);

	for (const String &dir : Directory::GetDirectories(curr))
	{
		Console::WriteLine(dir);
	}
	e1 = Directory::Exists(dirNew);
	REQUIRE(e1, == , false);
	Console::WriteLine(L"---");
}