#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;

//TEST_CASE(console_)
//{
    //int i = 0;
    //do
    //{
    //    Console::WriteLine(L"\nPress a key to display; press the 'x' key to quit.");
    //    while (!_kbhit()) Thread::Sleep(250);

    //    i = _getch();
    //    Console::WriteLine(L"You pressed the '{0}' key.", i);
    //} while (i != (int)'x');

    //

    //return 0;

    //ConsoleKeyInfo cki;

    //do {
    //    Console::WriteLine(L"\nPress a key to display; press the 'x' key to quit.");

    //    // Your code could perform some useful task in the following loop. However, 
    //    // for the sake of this example we'll merely pause for a quarter second.

    //    while (Console::KeyAvailable() == false)
    //        Thread::Sleep(250); // Loop until input is entered.
    //    cki = Console::ReadKey(true);
    //    Console::WriteLine(L"You pressed the '{0}' key.", cki.Key());
    //} while (cki.Key != ConsoleKey::X);
//}