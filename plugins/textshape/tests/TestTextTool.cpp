/*
 *  This file is part of KOffice tests
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
#include "TestTextTool.h"

#include "../TextTool.h"
#include "MockTextShape.h"

#include <tests/MockShapes.h>

void TestTextTool::testTextRect()
{
    TextTool tool(new MockCanvas());

    MockTextShape *shape1 = new MockTextShape();
    QVERIFY(shape1->userData());
    QVERIFY(qobject_cast<KoTextShapeData*>(shape1->userData()));

    tool.setShapeData(qobject_cast<KoTextShapeData*>(shape1->userData()));
    QVERIFY(tool.textEditor());

    QTextCursor cursor(qobject_cast<KoTextShapeData*>(shape1->userData())->document());
    cursor.insertText("foo\n");

    QCOMPARE(tool.textRect(0,0), QRectF()); // invalid
    QCOMPARE(tool.textRect(14,14), QRectF()); // invalid

    shape1->layout->layout();

    QCOMPARE(tool.textRect(0, 0).topLeft(), QPointF());
    // second line should be lower.
    //qDebug() << tool.textRect(4, 4);
    QEXPECT_FAIL("", "bug 213238", Abort);
    QVERIFY(tool.textRect(4, 4).topLeft().y() > 0);
}

QTEST_KDEMAIN(TestTextTool, GUI)

#include <TestTextTool.moc>
