/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestNode.h"
#include <simpletest.h>
#include <QColor>
#include <QDataStream>

#include <KritaVersionWrapper.h>
#include <Node.h>
#include <Krita.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_fill_painter.h>
#include <kis_paint_layer.h>

#include <sdk/tests/testui.h>

void TestNode::testSetColorSpace()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QStringList profiles = Krita().profiles("GRAYA", "U16");
    node->setColorSpace("GRAYA", "U16", profiles.first());
    QCOMPARE(layer->colorSpace()->colorModelId().id() , "GRAYA");
    QCOMPARE(layer->colorSpace()->colorDepthId().id() , "U16");
    QCOMPARE(layer->colorSpace()->profile()->name(), "Gray-D50-elle-V2-g10.icc");
}

void TestNode::testSetColorProfile()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QStringList profiles = Krita().profiles("RGBA", "U8");
    Q_FOREACH(const QString &profile, profiles) {
        node->setColorProfile(profile);
        QVERIFY(layer->colorSpace()->profile()->name() == profile);
    }
}

void TestNode::testPixelData()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QByteArray ba = node->pixelData(0, 0, 100, 100);
    QDataStream ds(ba);
    do {
        quint8 channelvalue;
        ds >> channelvalue;
        QVERIFY(channelvalue == 0);
        ds >> channelvalue;
        QVERIFY(channelvalue == 0);
        ds >> channelvalue;
        QVERIFY(channelvalue == 255);
        ds >> channelvalue;
        QVERIFY(channelvalue == 255);
    } while (!ds.atEnd());

    QDataStream ds2(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 100 * 100; i++) {
        ds2 << 255;
        ds2 << 255;
        ds2 << 255;
        ds2 << 255;
    }

    node->setPixelData(ba, 0, 0, 100, 100);
    for (int i = 0; i < 100 ; i++) {
        for (int j = 0; j < 100 ; j++) {
            QColor pixel;
            layer->paintDevice()->pixel(i, j, &pixel);
            QVERIFY(pixel == QColor(Qt::black));
        }
    }
}

void TestNode::testProjectionPixelData()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::gray, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QByteArray ba = node->projectionPixelData(0, 0, 100, 100);
    QDataStream ds(ba);
    for (int i = 0; i < 100 * 100; i++) {
        quint8 channelvalue;
        ds >> channelvalue;
        QVERIFY(channelvalue == 0xA4);
        ds >> channelvalue;
        QVERIFY(channelvalue == 0xA0);
        ds >> channelvalue;
        QVERIFY(channelvalue == 0xA0);
        ds >> channelvalue;
        QVERIFY(channelvalue == 0xFF);
    }
}

void TestNode::testThumbnail()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::gray, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QImage thumb = node->thumbnail(10, 10);
    thumb.save("thumb.png");
    QVERIFY(thumb.width() == 10);
    QVERIFY(thumb.height() == 10);
    // Our thumbnail calculator in KisPaintDevice cannot make a filled 10x10 thumbnail from a 100x100 device,
    // it makes it 10x10 empty, then puts 8x8 pixels in there... Not a bug in the Node class
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QVERIFY(thumb.pixelColor(i, j) == QColor(Qt::gray));
        }
    }
}

void TestNode::testMergeDown()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");

    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    {
        KisFillPainter gc(layer->paintDevice());
        gc.fillRect(0, 0, 100, 100, KoColor(Qt::gray, layer->colorSpace()));
    }
    image->addNode(layer);

    KisNodeSP layer2 = new KisPaintLayer(image, "test2", 255);
    {
        KisFillPainter gc(layer2->paintDevice());
        gc.fillRect(0, 0, 100, 100, KoColor(Qt::gray, layer2->colorSpace()));
    }
    image->addNode(layer2);
    NodeSP n1 = NodeSP(Node::createNode(image, layer));
    Node *n2 = n1->mergeDown();
    delete n2;
}

KISTEST_MAIN(TestNode)

