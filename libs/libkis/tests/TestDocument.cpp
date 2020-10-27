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
#include <QColor>
#include <QDataStream>
#include <QDir>

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

#include <sdk/tests/testui.h>

void TestDocument::testSetColorSpace()
{
    QScopedPointer<KisDocument> kisdoc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc.data(), false);
    QStringList profiles = Krita().profiles("GRAYA", "U16");
    d.setColorSpace("GRAYA", "U16", profiles.first());

    QVERIFY(layer->colorSpace()->colorModelId().id() == "GRAYA");
    QVERIFY(layer->colorSpace()->colorDepthId().id() == "U16");
    QVERIFY(layer->colorSpace()->profile()->name() == profiles.first());

    KisPart::instance()->removeDocument(kisdoc.data(), false);
}

void TestDocument::testSetColorProfile()
{
    QScopedPointer<KisDocument> kisdoc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc.data(), false);

    QStringList profiles = Krita().profiles("RGBA", "U8");
    Q_FOREACH(const QString &profileName, profiles) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(profileName);

        // skip input-only profiles (e.g. for scanners)
        if (!profile->isSuitableForOutput()) continue;

        d.setColorProfile(profileName);
        QVERIFY(image->colorSpace()->profile()->name() == profileName);
    }
    KisPart::instance()->removeDocument(kisdoc.data(), false);
}

void TestDocument::testPixelData()
{
    QScopedPointer<KisDocument> kisdoc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc.data(), false);
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

    KisPart::instance()->removeDocument(kisdoc.data(), false);
}

void TestDocument::testThumbnail()
{
    QScopedPointer<KisDocument> kisdoc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    KisNodeSP layer = new KisPaintLayer(image, "test1", 255);
    KisFillPainter gc(layer->paintDevice());
    gc.fillRect(0, 0, 100, 100, KoColor(Qt::red, layer->colorSpace()));
    image->addNode(layer);
    kisdoc->setCurrentImage(image);

    Document d(kisdoc.data(), false);
    d.refreshProjection();

    QImage thumb = d.thumbnail(10, 10);
    thumb.save("thumb.png");
    QVERIFY(thumb.width() == 10);
    QVERIFY(thumb.height() == 10);
    // Our thumbnail calculator in KisPaintDevice cannot make a filled 10x10 thumbnail from a 100x100 device,
    // it makes it 10x10 empty, then puts 8x8 pixels in there... Not a bug in the Node class
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QVERIFY(thumb.pixelColor(i, j) == QColor(Qt::red));
        }
    }
    KisPart::instance()->removeDocument(kisdoc.data(), false);
}

void TestDocument::testCreateFillLayer()
{
    QScopedPointer<KisDocument> kisdoc(KisPart::instance()->createDocument());
    KisImageSP image = new KisImage(0, 50, 50, KoColorSpaceRegistry::instance()->rgb16(), "test");
    kisdoc->setCurrentImage(image);
    Document d(kisdoc.data(), false);

    const QString pattern("pattern");
    const QString color("color");
    const QString filllayer = "filllayer";
    InfoObject info;
    Selection sel(image->globalSelection());

    FillLayer *f = d.createFillLayer("test1", pattern, info, sel);
    QVERIFY(f->generatorName() == pattern);
    QVERIFY(f->type() == filllayer);
    delete f;
    f = d.createFillLayer("test1", color, info, sel);
    QVERIFY(f->generatorName() == color);
    QVERIFY(f->type() == filllayer);

    info.setProperty(pattern, "Cross01.pat");
    QVERIFY(f->setGenerator(pattern, &info));
    QVERIFY(f->filterConfig()->property(pattern).toString() == "Cross01.pat");
    QVERIFY(f->generatorName() == pattern);
    QVERIFY(f->type() == filllayer);

    info.setProperty(color, QColor(Qt::red));
    QVERIFY(f->setGenerator(color, &info));
    QVariant v = f->filterConfig()->property(color);
    QColor c = v.value<QColor>();
    QVERIFY(c == QColor(Qt::red));
    QVERIFY(f->generatorName() == color);
    QVERIFY(f->type() == filllayer);

    bool r = f->setGenerator(QString("xxx"), &info);
    QVERIFY(!r);

    delete f;

    QVERIFY(d.createFillLayer("test1", "xxx", info, sel) == 0);

    KisPart::instance()->removeDocument(kisdoc.data(), false);
}



KISTEST_MAIN(TestDocument)

