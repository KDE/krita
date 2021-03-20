/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_view_converter_test.h"
#include <QApplication>

#include <kis_debug.h>
#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_image_view_converter.h"
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_types.h"

void KisImageViewConverterTest::testDocumentToView()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "test");
    KisImageViewConverter viewConverter(image);

    image->setResolution(1.38888888, 1.38888888);

    QVERIFY(viewConverter.documentToView(QPointF(0.0, 0.0))
            == QPointF(0.0, 0.0));

    QPointF f = viewConverter.documentToView(QPointF(3.2, 5.2));
    QVERIFY(f.x() < 4.44445 && f.x() > 4.44443 && f.y() < 7.22223 && f.y() > 7.22221);

    QRectF r = viewConverter.documentToView(QRectF(0.0, 0.0, 10.0, 10.0));
    QVERIFY(r.width() < 13.889 && r.width() > 13.8888
            && r.height() < 13.889 && r.height() > 13.8888);

    QSizeF s = viewConverter.documentToView(QSizeF(1.0, 1.0));
    QVERIFY(s.width() < 1.3888889 && s.width() > 1.388887
            && s.height() < 1.3888889 && s.height() > 1.388887);

    double x = viewConverter.documentToViewX(1.0);
    QVERIFY(x < 1.3888889 && x > 1.388887);

    double y = viewConverter.documentToViewY(1.0);
    QVERIFY(y < 1.3888889 && y > 1.388887);

}

void KisImageViewConverterTest::testViewToDocument()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "test");
    KisImageViewConverter viewConverter(image);

    image->setResolution(1.38888888, 1.38888888);

    QVERIFY(viewConverter.viewToDocument(QPointF(0.0, 0.0))
            == QPointF(0.0, 0.0));

    QPointF f = viewConverter.viewToDocument(QPointF(5, 5));
    QVERIFY(f.x() < 3.7 && f.x() > 3.5 && f.y() < 3.7 && f.y() > 3.5);

    QRectF r = viewConverter.viewToDocument(QRectF(0.0, 0.0, 5, 5));
    QVERIFY(r.width() < 3.7 && r.width() > 3.5
            && r.height() < 3.7 && r.height() > 3.5);

    QSizeF s = viewConverter.viewToDocument(QSizeF(1.0, 1.0));
    QVERIFY(s.width() < 0.721 && s.width() > 0.719
            && s.height() < 0.721 && s.height() > 0.719);

    double x = viewConverter.viewToDocumentX(1.0);
    QVERIFY(x < 0.721 && x > 0.719);

    double y = viewConverter.viewToDocumentY(1.0);
    QVERIFY(y < 0.721 && y > 0.719);

}

void KisImageViewConverterTest::testZoom()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "test");
    KisImageViewConverter viewConverter(image);

    image->setResolution(1.38888888, 5.38888888);

    qreal zoomX, zoomY;

    viewConverter.zoom(&zoomX, &zoomY);
    QVERIFY(zoomX < 1.388889 && zoomX > 1.3888887);
    QVERIFY(zoomY < 5.388889 && zoomY > 5.3888887);
}

SIMPLE_TEST_MAIN(KisImageViewConverterTest)
