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

#include "kis_integer_maths_tester.h"
#include "kis_integer_maths.h"

using namespace KUnitTest;

KUNITTEST_MODULE(kunittest_kis_integer_maths_tester, "Integer Maths Tester");
KUNITTEST_MODULE_REGISTER_TESTER(KisIntegerMathsTester);

void KisIntegerMathsTester::allTests()
{
    UINT8Tests();
    UINT16Tests();
    conversionTests();
}

void KisIntegerMathsTester::UINT8Tests()
{
    CHECK((int)UINT8_MULT(0, 255), 0);
    CHECK((int)UINT8_MULT(255, 255), 255);

    CHECK((int)UINT8_MULT(128, 255), 128);
    CHECK((int)UINT8_MULT(255, 128), 128);

    CHECK((int)UINT8_MULT(1, 255), 1);
    CHECK((int)UINT8_MULT(1, 127), 0);
    CHECK((int)UINT8_MULT(64, 128), 32);

    CHECK((int)UINT8_DIVIDE(255, 255), 255);
    CHECK((int)UINT8_DIVIDE(64, 128), 128);
    CHECK((int)UINT8_DIVIDE(1, 64), 4);
    CHECK((int)UINT8_DIVIDE(0, 1), 0);

    CHECK((int)UINT8_BLEND(255, 0, 0), 0);
    CHECK((int)UINT8_BLEND(255, 0, 128), 128);
    CHECK((int)UINT8_BLEND(255, 128, 128), 192);
    CHECK((int)UINT8_BLEND(128, 64, 255), 128);
}

void KisIntegerMathsTester::UINT16Tests()
{
    CHECK((int)UINT16_MULT(0, 65535), 0);
    CHECK((int)UINT16_MULT(65535, 65535), 65535);

    CHECK((int)UINT16_MULT(32768, 65535), 32768);
    CHECK((int)UINT16_MULT(65535, 32768), 32768);

    CHECK((int)UINT16_MULT(1, 65535), 1);
    CHECK((int)UINT16_MULT(1, 32767), 0);
    CHECK((int)UINT16_MULT(16384, 32768), 8192);

    CHECK((int)UINT16_DIVIDE(65535, 65535), 65535);
    CHECK((int)UINT16_DIVIDE(16384, 32768), 32768);
    CHECK((int)UINT16_DIVIDE(1, 16384), 4);
    CHECK((int)UINT16_DIVIDE(0, 1), 0);

    CHECK((int)UINT16_BLEND(65535, 0, 0), 0);
    CHECK((int)UINT16_BLEND(65535, 0, 32768), 32768);
    CHECK((int)UINT16_BLEND(65535, 32768, 32768), 49152);
    CHECK((int)UINT16_BLEND(32768, 16384, 65535), 32768);
}

void KisIntegerMathsTester::conversionTests()
{
    CHECK((int)UINT8_TO_UINT16(255), 65535);
    CHECK((int)UINT8_TO_UINT16(0), 0);
    CHECK((int)UINT8_TO_UINT16(128), 128 * 257);

    CHECK((int)UINT16_TO_UINT8(65535), 255);
    CHECK((int)UINT16_TO_UINT8(0), 0);
    CHECK((int)UINT16_TO_UINT8(128 * 257), 128);
}

