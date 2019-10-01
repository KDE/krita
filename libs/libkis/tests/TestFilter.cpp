/* Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

#include  <sdk/tests/kistest.h>

void TestFilter::testApply()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::black, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);
    Document d(kisdoc);
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
    Document d(kisdoc);
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

