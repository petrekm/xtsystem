#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;

TEST_CASE(path_GetDirectoryName)
{

    REQUIRE(IO::Path::GetDirectoryName(_T("C:\\MyDir\\MySubDir\\*.ext")), ==, _T("C:\\MyDir\\MySubDir"));
    REQUIRE(IO::Path::GetDirectoryName(_T("C:\\MyDir\\MySubDir\\myfile.ext")), ==, _T("C:\\MyDir\\MySubDir"));
    REQUIRE(IO::Path::GetDirectoryName(_T("C:\\MyDir\\MySubDir\\")), ==, _T("C:\\MyDir\\MySubDir"));
    REQUIRE(IO::Path::GetDirectoryName(_T("C:\\MyDir\\MySubDir")), ==, _T("C:\\MyDir"));
    REQUIRE(IO::Path::GetDirectoryName(_T("C:\\")), ==, _T(""));

	REQUIRE(IO::Path::GetDirectoryName(_T("C:/MyDir/MySubDir/*.ext")), ==, _T("C:/MyDir/MySubDir"));
    REQUIRE(IO::Path::GetDirectoryName(_T("C:/MyDir\\MySubDir\\")), ==, _T("C:/MyDir\\MySubDir"));

	REQUIRE(IO::Path::Combine(_T(""),_T("")), ==, _T(""));
	REQUIRE(IO::Path::Combine(_T("A"),_T("")), ==, _T("A"));
	REQUIRE(IO::Path::Combine(_T(""),_T("B")), ==, _T("B"));
	REQUIRE(IO::Path::Combine(_T("A"),_T("B")), ==, _T("A\\B"));
	REQUIRE(IO::Path::Combine(_T("A\\"),_T("B")), ==, _T("A\\B"));
	REQUIRE(IO::Path::Combine(_T("A/"),_T("B")), ==, _T("A/B"));
	REQUIRE(IO::Path::Combine(_T("A"),_T("\\B")), ==, _T("A\\B"));
	REQUIRE(IO::Path::Combine(_T("A"),_T("/B")), ==, _T("A/B"));
	REQUIRE(IO::Path::Combine(_T("A\\"),_T("/B")), ==, _T("A\\B"));
	REQUIRE(IO::Path::Combine(_T("A/"),_T("\\B")), ==, _T("A\\B"));
	REQUIRE(IO::Path::Combine(_T("C:\\a"),_T("D:\\B")), ==, _T("D:\\B"));
	REQUIRE(IO::Path::Combine(_T("C:\\a"),_T("test.txt")), ==, _T("C:\\a\\test.txt"));

    REQUIRE(IO::Path::GetFileName(_T("C:\\MyDir\\MySubDir\\*.ext")), ==, _T("*.ext"));
    REQUIRE(IO::Path::GetFileName(_T("C:\\MyDir\\MySubDir\\myfile.ext")), ==, _T("myfile.ext"));
    REQUIRE(IO::Path::GetFileName(_T("C:\\MyDir\\MySubDir\\")), ==, _T(""));
    REQUIRE(IO::Path::GetFileName(_T("C:\\MyDir\\MySubDir")), ==, _T("MySubDir"));
 
}
