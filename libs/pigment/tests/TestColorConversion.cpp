/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestColorConversion.h"
#include "KoColorConversions.h"

#include <QTest>

void TestColorConversion::testRGBHSV()
{
    float r, g, b, h, s, v;

    RGBToHSV(1, 0, 0, &h, &s, &v);
    QCOMPARE(h, 0.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(1, 1, 0, &h, &s, &v);
    QCOMPARE(h, 60.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(0, 1, 0, &h, &s, &v);
    QCOMPARE(h, 120.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(0, 1, 1, &h, &s, &v);
    QCOMPARE(h, 180.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(0, 0, 1, &h, &s, &v);
    QCOMPARE(h, 240.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(1, 0, 1, &h, &s, &v);
    QCOMPARE(h, 300.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(0, 0, 0, &h, &s, &v);
    QCOMPARE(h, -1.0f);
    QCOMPARE(s, 0.0f);
    QCOMPARE(v, 0.0f);

    RGBToHSV(1, 1, 1, &h, &s, &v);
    QCOMPARE(h, -1.0f);
    QCOMPARE(s, 0.0f);
    QCOMPARE(v, 1.0f);

    RGBToHSV(0.5, 0.25, 0.75, &h, &s, &v);
    QCOMPARE(h, 270.0f);
    QCOMPARE(s, 0.666667f);
    QCOMPARE(v, 0.75f);

    HSVToRGB(0, 1, 1, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 0.0f);

    HSVToRGB(60, 1, 1, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 0.0f);

    HSVToRGB(120, 1, 1, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 0.0f);

    HSVToRGB(180, 1, 1, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 1.0f);

    HSVToRGB(240, 1, 1, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 1.0f);

    HSVToRGB(300, 1, 1, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 1.0f);

    HSVToRGB(-1, 0, 0, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 0.0f);

    HSVToRGB(-1, 0, 1, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 1.0f);

    HSVToRGB(270, 0.666667f, 0.75f, &r, &g, &b);
    QCOMPARE(r, 0.5f);
    QCOMPARE(g, 0.25f);
    QCOMPARE(b, 0.75f);
}

void TestColorConversion::testRGBHSL()
{
    float r, g, b, h, s, l;

    RGBToHSL(1, 0, 0, &h, &s, &l);
    QCOMPARE(h, 0.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(1, 1, 0, &h, &s, &l);
    QCOMPARE(h, 60.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(0, 1, 0, &h, &s, &l);
    QCOMPARE(h, 120.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(0, 1, 1, &h, &s, &l);
    QCOMPARE(h, 180.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(0, 0, 1, &h, &s, &l);
    QCOMPARE(h, 240.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(1, 0, 1, &h, &s, &l);
    QCOMPARE(h, 300.0f);
    QCOMPARE(s, 1.0f);
    QCOMPARE(l, 0.5f);

    RGBToHSL(0, 0, 0, &h, &s, &l);
    QCOMPARE(h, -1.0f);
    QCOMPARE(s, 0.0f);
    QCOMPARE(l, 0.0f);

    RGBToHSL(1, 1, 1, &h, &s, &l);
    QCOMPARE(h, -1.0f);
    QCOMPARE(s, 0.0f);
    QCOMPARE(l, 1.0f);

    RGBToHSL(0.5, 0.25, 0.75, &h, &s, &l);
    QCOMPARE(h, 270.0f);
    QCOMPARE(s, 0.5f);
    QCOMPARE(l, 0.5f);

    HSLToRGB(0, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 0.0f);

    HSLToRGB(60, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 0.0f);

    HSLToRGB(120, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 0.0f);

    HSLToRGB(180, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 1.0f);

    HSLToRGB(240, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 1.0f);

    HSLToRGB(300, 1, 0.5, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 1.0f);

    HSLToRGB(-1, 0, 0, &r, &g, &b);
    QCOMPARE(r, 0.0f);
    QCOMPARE(g, 0.0f);
    QCOMPARE(b, 0.0f);

    HSLToRGB(-1, 0, 1, &r, &g, &b);
    QCOMPARE(r, 1.0f);
    QCOMPARE(g, 1.0f);
    QCOMPARE(b, 1.0f);

    HSLToRGB(270, 0.5, 0.5, &r, &g, &b);
    QCOMPARE(r, 0.5f);
    QCOMPARE(g, 0.25f);
    QCOMPARE(b, 0.75f);
}

QTEST_GUILESS_MAIN(TestColorConversion)
