/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "TestSvgText.h"

#include <QTest>

#include "SvgParserTestingUtils.h"

void TestSvgText::testSimpleText()
{
    const QString data =
            "<svg width=\"100px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"




            "<g id=\"testRect\">"

            "    <rect id=\"boundingRect\" x=\"5\" y=\"5\" width=\"89\" height=\"19\""
            "        fill=\"none\" stroke=\"red\"/>"

            "    <text x=\"7\" y=\"7\""
            "        font-family=\"Verdana\" font-size=\"15\" fill=\"blue\" >"
            "        Hello, out there"
            "    </text>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("test_simple_text", false, QSize(100, 30));
}

QTEST_MAIN(TestSvgText)
