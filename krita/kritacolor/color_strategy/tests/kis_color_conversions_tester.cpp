/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_color_conversions_tester.h"
#include "kis_color_conversions.h"

using namespace KUnitTest;

KUNITTEST_MODULE(kunittest_kis_color_conversions_tester, "Color Conversions Tester");
KUNITTEST_MODULE_REGISTER_TESTER(KisColorConversionsTester);

void KisColorConversionsTester::allTests()
{
    testRGBHSV();
    testRGBHSL();
}

#define EPSILON 1e-6

void KisColorConversionsTester::testRGBHSV()
{
    float r, g, b, h, s, v;

    RGBToHSV(1, 0, 0, &h, &s, &v);
    CHECK(h, 0.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(1, 1, 0, &h, &s, &v);
    CHECK(h, 60.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(0, 1, 0, &h, &s, &v);
    CHECK(h, 120.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(0, 1, 1, &h, &s, &v);
    CHECK(h, 180.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(0, 0, 1, &h, &s, &v);
    CHECK(h, 240.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(1, 0, 1, &h, &s, &v);
    CHECK(h, 300.0f);
    CHECK(s, 1.0f);
    CHECK(v, 1.0f);

    RGBToHSV(0, 0, 0, &h, &s, &v);
    CHECK(h, -1.0f);
    CHECK(s, 0.0f);
    CHECK(v, 0.0f);

    RGBToHSV(1, 1, 1, &h, &s, &v);
    CHECK(h, -1.0f);
    CHECK(s, 0.0f);
    CHECK(v, 1.0f);

    RGBToHSV(0.5, 0.25, 0.75, &h, &s, &v);
    CHECK_TOLERANCE(h, 270.0f, EPSILON);
    CHECK_TOLERANCE(s, 0.666667f, EPSILON);
    CHECK_TOLERANCE(v, 0.75f, EPSILON);

    HSVToRGB(0, 1, 1, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 0.0f);
    CHECK(b, 0.0f);

    HSVToRGB(60, 1, 1, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 1.0f);
    CHECK(b, 0.0f);

    HSVToRGB(120, 1, 1, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 1.0f);
    CHECK(b, 0.0f);

    HSVToRGB(180, 1, 1, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 1.0f);
    CHECK(b, 1.0f);

    HSVToRGB(240, 1, 1, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 0.0f);
    CHECK(b, 1.0f);

    HSVToRGB(300, 1, 1, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 0.0f);
    CHECK(b, 1.0f);

    HSVToRGB(-1, 0, 0, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 0.0f);
    CHECK(b, 0.0f);

    HSVToRGB(-1, 0, 1, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 1.0f);
    CHECK(b, 1.0f);

    HSVToRGB(270, 0.666667, 0.75, &r, &g, &b);
    CHECK_TOLERANCE(r, 0.5f, EPSILON);
    CHECK_TOLERANCE(g, 0.25f, EPSILON);
    CHECK_TOLERANCE(b, 0.75f, EPSILON);
}

void KisColorConversionsTester::testRGBHSL()
{
    float r, g, b, h, s, l;

    RGBToHSL(1, 0, 0, &h, &s, &l);
    CHECK(h, 360.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(1, 1, 0, &h, &s, &l);
    CHECK(h, 60.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(0, 1, 0, &h, &s, &l);
    CHECK(h, 120.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(0, 1, 1, &h, &s, &l);
    CHECK(h, 180.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(0, 0, 1, &h, &s, &l);
    CHECK(h, 240.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(1, 0, 1, &h, &s, &l);
    CHECK(h, 300.0f);
    CHECK(s, 1.0f);
    CHECK(l, 0.5f);

    RGBToHSL(0, 0, 0, &h, &s, &l);
    CHECK(h, -1.0f);
    CHECK(s, 0.0f);
    CHECK(l, 0.0f);

    RGBToHSL(1, 1, 1, &h, &s, &l);
    CHECK(h, -1.0f);
    CHECK(s, 0.0f);
    CHECK(l, 1.0f);

    RGBToHSL(0.5, 0.25, 0.75, &h, &s, &l);
    CHECK_TOLERANCE(h, 270.0f, EPSILON);
    CHECK_TOLERANCE(s, 0.5f, EPSILON);
    CHECK_TOLERANCE(l, 0.5f, EPSILON);

    HSLToRGB(0, 1, 0.5, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 0.0f);
    CHECK(b, 0.0f);

    HSLToRGB(60, 1, 0.5, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 1.0f);
    CHECK(b, 0.0f);

    HSLToRGB(120, 1, 0.5, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 1.0f);
    CHECK(b, 0.0f);

    HSLToRGB(180, 1, 0.5, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 1.0f);
    CHECK(b, 1.0f);

    HSLToRGB(240, 1, 0.5, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 0.0f);
    CHECK(b, 1.0f);

    HSLToRGB(300, 1, 0.5, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 0.0f);
    CHECK(b, 1.0f);

    HSLToRGB(-1, 0, 0, &r, &g, &b);
    CHECK(r, 0.0f);
    CHECK(g, 0.0f);
    CHECK(b, 0.0f);

    HSLToRGB(-1, 0, 1, &r, &g, &b);
    CHECK(r, 1.0f);
    CHECK(g, 1.0f);
    CHECK(b, 1.0f);

    HSLToRGB(270, 0.5, 0.5, &r, &g, &b);
    CHECK_TOLERANCE(r, 0.5f, EPSILON);
    CHECK_TOLERANCE(g, 0.25f, EPSILON);
    CHECK_TOLERANCE(b, 0.75f, EPSILON);
}

