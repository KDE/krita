/* This file is part of the KOffice project
 * Copyright (C) 2010 KO GmbH <casper.boemann@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestSectionStyle.h"

#include "styles/KoSectionStyle.h"

#include <QTextFrameFormat>

void TestSectionStyle::testSectionStyle()
{
    // Test basic functionality of the section style (roundtripping to format and back).
    KoSectionStyle *style1 = new KoSectionStyle();
    QVERIFY(style1);
    style1->setLeftMargin(1.0);
    style1->setRightMargin(2.0);

    QTextFrameFormat format;
    style1->applyStyle(format);

    KoSectionStyle *style2 = new KoSectionStyle(format);
    QVERIFY(style2);
    QCOMPARE(style2->leftMargin(), 1.0);
    QCOMPARE(style2->rightMargin(), 2.0);

}

QTEST_MAIN(TestSectionStyle)
#include <TestSectionStyle.moc>
