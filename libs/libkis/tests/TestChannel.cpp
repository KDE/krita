/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestChannel.h"
#include <QTest>
#include <QColor>
#include <QDataStream>
#include <QLoggingCategory>

#include <KritaVersionWrapper.h>
#include <Node.h>
#include <Channel.h>
#include <Krita.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_fill_painter.h>
#include <kis_paint_layer.h>
#include "sdk/tests/testui.h"


void TestChannel::testPixelDataU8()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QList<Channel*> channels = node->channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 1);
    }

}

void TestChannel::testPixelDataU16()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb16(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QList<Channel*> channels = node->channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 2);
    }
}

void TestChannel::testPixelDataF16()
{
#ifdef HAVE_OPENEXR
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F16", "");
    KisImageSP image = new KisImage(0, 100, 100, cs, "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QList<Channel*> channels = node->channels();
    Q_FOREACH(Channel *channel, channels) {
        qDebug() << "channelsize" << channel->channelSize();
        QVERIFY(channel->channelSize() == 2);
    }
#endif
}

void TestChannel::testPixelDataF32()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", ""), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QList<Channel*> channels = node->channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 4);
    }
}

void TestChannel::testReadWritePixelData()
{
    KisImageSP image = new KisImage(0, 2, 2, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "U8", ""), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 2, 2, KoColor(Qt::yellow, layer->colorSpace()));
    NodeSP node = NodeSP(Node::createNode(image, layer));
    QList<Channel*> channels = node->channels();
    Channel *greenChan = channels[1];
    QVERIFY(greenChan->name() == "Green");
    QRect rc = greenChan->bounds();
    QVERIFY(rc == QRect(0, 0, 2, 2));
    QByteArray ba = greenChan->pixelData(rc);
    ba.fill('\x80', 4);
    greenChan->setPixelData(ba, rc);
    image->refreshGraph();
    QColor c;
    layer->paintDevice()->pixel(0, 0, &c);
    QVERIFY(c == QColor(255, 128, 0));

}


KISTEST_MAIN(TestChannel)

