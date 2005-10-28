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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef KIS_STRATEGY_COLORSPACE_RGB_F32_TESTER_H
#define KIS_STRATEGY_COLORSPACE_RGB_F32_TESTER_H

#include <kunittest/tester.h>

#define CHECK_TOLERANCE( x, y, tolerance ) \
if ((x) <= (y) + (tolerance) && (x) >= (y) - (tolerance)) \
{ \
    success(QString(__FILE__) + "[" + QString::number(__LINE__) + "]: passed " + #x); \
} \
else \
{ \
    failure(QString(__FILE__) + "[" + QString::number(__LINE__) + QString("]: failed ") + #x + "\n Expected " + #y + ", Actual result " + QString::number(x)); \
} \

class KisRgbF32ColorSpaceTester : public KUnitTest::Tester
{
public:
        void allTests();
    void testBasics();
    void testMixColors();
    void testToQImage();
    void testCompositeOps();
};

#endif

