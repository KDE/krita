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
#include "TestDocument.h"
#include <QTest>

#include <KritaVersionWrapper.h>
#include <QTest>
#include <QColor>
#include <QDataStream>
#include <QDir>

#include <KritaVersionWrapper.h>
#include <Node.h>
#include <Krita.h>
#include <Document.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColor.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_fill_painter.h>
#include <kis_paint_layer.h>
#include <KisPart.h>

void TestDocument::testSetColorSpace()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc);
    QStringList profiles = Krita().profiles("GRAYA", "U16");
    d.setColorSpace("GRAYA", "U16", profiles.first());

    QVERIFY(layer->colorSpace()->colorModelId().id() == "GRAYA");
    QVERIFY(layer->colorSpace()->colorDepthId().id() == "U16");
    QVERIFY(layer->colorSpace()->profile()->name() == "gray built-in");
}

void TestDocument::testSetColorProfile()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc);

    QStringList profiles = Krita().profiles("RGBA", "U8");
    Q_FOREACH(const QString &profile, profiles) {
        d.setColorProfile(profile);
        QVERIFY(image->colorSpace()->profile()->name() == profile);
    }
}

void TestDocument::testPixelData()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc);
    d.refreshProjection();

    QByteArray ba = d.pixelData(0, 0, 100, 100);
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
}

void TestDocument::testThumbnail()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc);
    d.refreshProjection();

    QImage thumb = d.thumbnail(10, 10);
    thumb.save("thumb.png");
    QVERIFY(thumb.width() == 10);
    QVERIFY(thumb.height() == 10);
    // Our thumbnail calculater in KisPaintDevice cannot make a filled 10x10 thumbnail from a 100x100 device,
    // it makes it 10x10 empty, then puts 8x8 pixels in there... Not a bug in the Node class
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
            QVERIFY(thumb.pixelColor(i, j) == QColor(Qt::red));
#else
            QVERIFY(QColor(thumb.pixel(i, j)) == QColor(Qt::red));
#endif
        }
    }

}

void TestDocument::testCreateAndSave()
{
    KisDocument *kisdoc = KisPart::instance()->createDocument();
    KisImageSP image = new KisImage(0, 5000, 5000, KoColorSpaceRegistry::instance()->rgb16(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 128);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(100, 100, 4000, 4000, KoColor(Qt::red, layer->colorSpace()));
    image->addNode(layer);
    for(int i = 0; i < 10; ++i) {
        image->addNode(layer->clone());
    }
    kisdoc->setCurrentImage(image);

    Document d(kisdoc);
    d.setBatchmode(true);
    d.refreshProjection();

    QString filename = QDir::tempPath() + "/TestDocumentTestCreateAndSave.kra";

    d.saveAs(filename);
    QVERIFY2(QFileInfo(filename).exists(), filename.toUtf8());

    Document *d2 = Krita::instance()->openDocument(filename);
    Q_ASSERT(d2->colorDepth() == "U16");

}



QTEST_MAIN(TestDocument)

