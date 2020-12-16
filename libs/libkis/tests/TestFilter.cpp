/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestFilter.h"
#include <QTest>

#include <KritaVersionWrapper.h>
#include <QColor>
#include <QDataStream>

#include <Node.h>
#include <Krita.h>
#include <Document.h>
#include <Filter.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_fill_painter.h>
#include <kis_paint_layer.h>
#include <KisPart.h>

#include  <sdk/tests/testui.h>

void TestFilter::testApply()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::black, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);
    Document d(kisdoc, false);
    NodeSP node = NodeSP(Node::createNode(image, layer));

    Filter f;
    f.setName("invert");
    QVERIFY(f.configuration());

    d.lock();
    f.apply(node.data(), 0, 0, 100, 100);
    d.unlock();
    d.refreshProjection();

    for (int i = 0; i < 100 ; i++) {
        for (int j = 0; j < 100 ; j++) {
            QColor pixel;
            layer->paintDevice()->pixel(i, j, &pixel);
            QVERIFY(pixel == QColor(Qt::white));
        }
    }

}

void TestFilter::testStartFilter()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::black, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);
    Document d(kisdoc, false);
    NodeSP node = NodeSP(Node::createNode(image, layer));

    Filter f;
    f.setName("invert");
    QVERIFY(f.configuration());

    f.startFilter(node.data(), 0, 0, 100, 100);
    image->waitForDone();

    for (int i = 0; i < 100 ; i++) {
        for (int j = 0; j < 100 ; j++) {
            QColor pixel;
            layer->paintDevice()->pixel(i, j, &pixel);
            QVERIFY(pixel == QColor(Qt::white));
        }
    }
}

KISTEST_MAIN(TestFilter)

