/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#include "TestBorder.h"

#include "styles/KoParagraphStyle.h"
#include "KoTextBlockBorderData.h"

#include <QTest>

void TestBorder::testBorder()
{
    KoParagraphStyle style;
    style.setLeftBorderWidth(4.0);
    style.setLeftBorderSpacing(2.0);
    style.setLeftInnerBorderWidth(1.0);
    style.setTopBorderWidth(6.0);
    style.setTopBorderSpacing(7.0);
    style.setTopInnerBorderWidth(8.0);
    QTextBlockFormat format;
    style.applyStyle(format);

    KoTextBlockBorderData data(QRectF(10, 10, 100, 100));

    data.setEdge(KoTextBlockBorderData::Left, format, KoParagraphStyle::LeftBorderStyle,
                 KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
                 KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    data.setEdge(KoTextBlockBorderData::Top, format, KoParagraphStyle::TopBorderStyle,
                 KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
                 KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);

    //QCOMPARE(QRectF(10, 10, 100, 80), data.rect());
    QCOMPARE(7., data.inset(KoTextBlockBorderData::Left));
    QCOMPARE(0., data.inset(KoTextBlockBorderData::Right));
    QCOMPARE(21., data.inset(KoTextBlockBorderData::Top));
    QCOMPARE(0., data.inset(KoTextBlockBorderData::Bottom));

}

QTEST_MAIN(TestBorder)
