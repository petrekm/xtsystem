#include "stdafx.h"
#include "Testing.h"
#include "XTSystem_all.h"

using namespace testing;
using namespace XTSystem;
using namespace XTSystem::Drawing;

TEST_CASE(drawing_basic)
{

    Color c = Color::AliceBlue;
    c = Color::Red;
    c = Color::Green;
    c = Color::Blue;

    Bitmap b(100, 200);
    REQUIRE(b.Width(), == , 100);
    REQUIRE(b.Height(), == , 200);

    b.SetPixel(10, 20, Color::Green);
    b.SetPixel(20, 30, Color::Blue);
    //REQUIRE(b.GetPixel(10, 20), == , Color::Green);
    //REQUIRE(b.GetPixel(20, 30), == , Color::Blue);

    //b.Save(L"C:\\a.pgm");

}

TEST_CASE(drawing_lines)
{
    Bitmap b(320, 200);
    Graphics g = Graphics::FromImage(b);

    g.Clear(Color::Gold);
    Pen pen(Color::Red);

    int x0 = 320 / 2;
    int y0 = 200 / 2;
    for (int phi = 0; phi < 360; phi += 10)
    {
	    double phi_d = Math::Deg2Rad(phi);
	    int x = x0 + (int)Math::Round(Math::Cos(phi_d) * 100);
        int y = y0 + (int)Math::Round(Math::Sin(phi_d) * 100);
	    g.DrawLine(pen, x0, y0, x, y);
    }

    //b.SetPixel(3, 4, Color::Green);
    //b.SetPixel(8, 10, Color::Blue);
    b.Save(L"a.pgm");
    b.Save(L"a.ppm");
}