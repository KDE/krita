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

#include "TestKoDrag.h"

#include <KoDrag.h>
#include <KoSvgPaste.h>

#include <kis_debug.h>
#include <kis_global.h>
#include <svg/SvgParser.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeGroup.h>

#include "../../sdk/tests/qimage_test_util.h"

void TestKoDrag::test()
{
    const QString fileName = TestUtil::fetchDataFileLazy("test_svg_file.svg");
    QVERIFY(!fileName.isEmpty());

    QFile testShapes(fileName);
    testShapes.open(QIODevice::ReadOnly);

    KoXmlDocument doc = SvgParser::createDocumentFromSvg(&testShapes);

    KoDocumentResourceManager resourceManager;
    SvgParser parser(&resourceManager);
    parser.setResolution(QRectF(0, 0, 30, 30) /* px */, 72 /* ppi */);

    QSizeF fragmentSize;
    QList<KoShape*> shapes = parser.parseSvg(doc.documentElement(), &fragmentSize);
    QCOMPARE(fragmentSize, QSizeF(30,30));

    {
        QCOMPARE(shapes.size(), 1);

        KoShapeGroup *layer = dynamic_cast<KoShapeGroup*>(shapes.first());
        QVERIFY(layer);
        QCOMPARE(layer->shapeCount(), 2);

        QCOMPARE(KoShape::absoluteOutlineRect(shapes).toAlignedRect(), QRect(6,6,19,18));
    }

    KoDrag drag;
    drag.setSvg(shapes);
    drag.addToClipboard();

    KoSvgPaste paste;
    QVERIFY(paste.hasShapes());

    QList<KoShape*> newShapes = paste.fetchShapes(QRectF(0,0,15,15) /* px */, 144 /* ppi */, &fragmentSize);

    {
        QCOMPARE(newShapes.size(), 1);

        KoShapeGroup *layer = dynamic_cast<KoShapeGroup*>(newShapes.first());
        QVERIFY(layer);
        QCOMPARE(layer->shapeCount(), 2);

        QCOMPARE(fragmentSize.toSize(), QSize(62, 61));
        QCOMPARE(KoShape::absoluteOutlineRect(newShapes).toAlignedRect(), QRect(6,6,19,18));
    }


    qDeleteAll(shapes);
    qDeleteAll(newShapes);
}


QTEST_MAIN(TestKoDrag)
