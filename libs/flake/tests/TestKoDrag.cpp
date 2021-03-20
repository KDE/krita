/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

        QCOMPARE(fragmentSize.toSize(), QSize(57, 55));
        QCOMPARE(KoShape::absoluteOutlineRect(newShapes).toAlignedRect(), QRect(6,6,19,18));
    }


    qDeleteAll(shapes);
    qDeleteAll(newShapes);
}


SIMPLE_TEST_MAIN(TestKoDrag)
