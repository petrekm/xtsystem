#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTSystem::IO;


TEST_CASE(io_binary_out)
{
    BinaryWriter bw;
    bw.Open(L"C:\\a1.dat");
    //bw.Write(4);
    //bw.Write(L"01");
    //bw.Write(L"Ažµ");

    //bw.Write(L"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");

    //bw.Write(3.14);
    //bw.Write(true);
    //bw.Write(false);
    bw.Close();
}