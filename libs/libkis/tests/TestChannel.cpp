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
#include "TestChannel.h"
#include <QTest>
#include <QColor>
#include <QDataStream>

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

void TestChannel::testPixelDataU8()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    Node node(image, layer);
    QList<Channel*> channels = node.channels();
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
    Node node(image, layer);
    QList<Channel*> channels = node.channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 2);
    }
}

void TestChannel::testPixelDataF16()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F16", ""), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    Node node(image, layer);
    QList<Channel*> channels = node.channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 2);
    }
}

void TestChannel::testPixelDataF32()
{
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", ""), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    Node node(image, layer);
    QList<Channel*> channels = node.channels();
    Q_FOREACH(Channel *channel, channels) {
        QVERIFY(channel->channelSize() == 4);
    }
}


QTEST_MAIN(TestChannel)

