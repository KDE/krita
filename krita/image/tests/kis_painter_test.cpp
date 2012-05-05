/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_painter_test.h"
#include <qtest_kde.h>


#include <kis_debug.h>
#include <QRect>
#include <QTime>
#include <QtXml>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_datamanager.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include <kis_fixed_paint_device.h>
#include "testutil.h"
#include <kis_iterator_ng.h>

void KisPainterTest::allCsApplicator(void (KisPainterTest::* funcPtr)(const KoColorSpace*cs))
{
    QList<const KoColorSpace*> colorsapces = KoColorSpaceRegistry::instance()->allColorSpaces(KoColorSpaceRegistry::AllColorSpaces, KoColorSpaceRegistry::OnlyDefaultProfile);

    foreach(const KoColorSpace* cs, colorsapces) {

        QString csId = cs->id();
        // ALL THESE COLORSPACES ARE BROKEN: WE NEED UNITTESTS FOR COLORSPACES!
        if (csId.startsWith("KS")) continue;
        if (csId.startsWith("Xyz")) continue;
        if (csId.startsWith('Y')) continue;
        if (csId.contains("AF")) continue;
        if (csId == "GRAYU16") continue; // No point in testing bounds with a cs without alpha
        if (csId == "GRAYU8") continue; // No point in testing bounds with a cs without alpha

        qDebug() << "Testing with cs" << csId;

        if (cs && cs->compositeOp(COMPOSITE_OVER) != 0) {
            (this->*funcPtr)(cs);
        } else {
            qDebug() << "Cannot bitBlt for cs" << csId;
        }
    }
}

void KisPainterTest::testSimpleBlt(const KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KoColor c(Qt::red, cs);
    c.setOpacity(quint8(128));
    src->fill(20, 20, 20, 20, c.data());

    QCOMPARE(src->exactBounds(), QRect(20, 20, 20, 20));

    const KoCompositeOp* op;

    {
        op = cs->compositeOp(COMPOSITE_OVER);
        KisPainter painter(dst);
        painter.setCompositeOp(op);
        painter.bitBlt(50, 50, src, 20, 20, 20, 20);
        painter.end();
        QCOMPARE(dst->exactBounds(), QRect(50,50,20,20));
    }

    dst->clear();

    {
        op = cs->compositeOp(COMPOSITE_COPY);
        KisPainter painter(dst);
        painter.setCompositeOp(op);
        painter.bitBlt(50, 50, src, 20, 20, 20, 20);
        painter.end();
        QCOMPARE(dst->exactBounds(), QRect(50,50,20,20));
    }
}

void KisPainterTest::testSimpleBlt()
{
    allCsApplicator(&KisPainterTest::testSimpleBlt);
}

/*

Note: the bltSelection tests assume the following geometry:

0,0               0,30
  +---------+------+
  |  10,10  |      |
  |    +----+      |
  |    |####|      |
  |    |####|      |
  +----+----+      |
  |       20,20    |
  |                |
  |                |
  +----------------+
                  30,30
 */
void KisPainterTest::testPaintDeviceBltSelection(const KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KoColor c(Qt::red, cs);
    c.setOpacity(quint8(128));
    src->fill(0, 0, 20, 20, c.data());

    QCOMPARE(src->exactBounds(), QRect(0, 0, 20, 20));

    KisSelectionSP selection = new KisSelection();
    selection->getOrCreatePixelSelection()->select(QRect(10, 10, 20, 20));
    selection->updateProjection();
    QCOMPARE(selection->selectedExactRect(), QRect(10, 10, 20, 20));

    KisPainter painter(dst);
    painter.setSelection(selection);

    painter.bitBlt(0, 0, src, 0, 0, 30, 30);
    painter.end();

    QImage image = dst->convertToQImage(0);
    image.save("blt_Selection_" + cs->name() + ".png");

    QCOMPARE(dst->exactBounds(), QRect(10, 10, 10, 10));

    const KoCompositeOp* op = cs->compositeOp(COMPOSITE_SUBTRACT);
    if (op->id() == COMPOSITE_SUBTRACT) {

        KisPaintDeviceSP dst2 = new KisPaintDevice(cs);
        KisPainter painter2(dst2);
        painter2.setSelection(selection);
        painter2.setCompositeOp(op);
        painter2.bitBlt(0, 0, src, 0, 0, 30, 30);
        painter2.end();

        QCOMPARE(dst2->exactBounds(), QRect(10, 10, 10, 10));
    }
}

void KisPainterTest::testPaintDeviceBltSelection()
{
    allCsApplicator(&KisPainterTest::testPaintDeviceBltSelection);
}

void KisPainterTest::testPaintDeviceBltSelectionIrregular(const KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisFillPainter gc(src);
    gc.fillRect(0, 0, 20, 20, KoColor(Qt::red, cs));
    gc.end();

    QCOMPARE(src->exactBounds(), QRect(0, 0, 20, 20));

    KisPixelSelectionSP psel = new KisPixelSelection();
    psel->select(QRect(10, 15, 20, 15));
    psel->select(QRect(15, 10, 15, 5));

    QCOMPARE(psel->selectedExactRect(), QRect(10, 10, 20, 20));

    QCOMPARE(TestUtil::alphaDevicePixel(psel, 13, 13), MIN_SELECTED);
    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(psel);
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bitBlt(0, 0, src, 0, 0, 30, 30);
    painter.end();

    QImage image = dst->convertToQImage(0);
    image.save("blt_Selection_irregular" + cs->name() + ".png");

    QCOMPARE(dst->exactBounds(), QRect(10, 10, 10, 10));
    foreach(KoChannelInfo * channel, cs->channels()) {
        // Only compare alpha if there actually is an alpha channel in
        // this colorspace
        if (channel->channelType() == KoChannelInfo::ALPHA) {
            QColor c;

            dst->pixel(13, 13, &c);

            QCOMPARE((int) c.alpha(), (int) OPACITY_TRANSPARENT_U8);
        }
    }
}


void KisPainterTest::testPaintDeviceBltSelectionIrregular()
{
    allCsApplicator(&KisPainterTest::testPaintDeviceBltSelectionIrregular);
}

void KisPainterTest::testPaintDeviceBltSelectionInverted(const KoColorSpace * cs)
{

    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisFillPainter gc(src);
    gc.fillRect(0, 0, 30, 30, KoColor(Qt::red, cs));
    gc.end();
    QCOMPARE(src->exactBounds(), QRect(0, 0, 30, 30));

    KisSelectionSP sel = new KisSelection();
    KisPixelSelectionSP psel = sel->getOrCreatePixelSelection();
    psel->select(QRect(10, 10, 20, 20));
    psel->invert();
    sel->updateProjection();

    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bitBlt(0, 0, src, 0, 0, 30, 30);
    painter.end();
    QCOMPARE(dst->exactBounds(), QRect(0, 0, 30, 30));
}

void KisPainterTest::testPaintDeviceBltSelectionInverted()
{
    allCsApplicator(&KisPainterTest::testPaintDeviceBltSelectionInverted);
}


void KisPainterTest::testSelectionBltSelection()
{
    KisPixelSelectionSP src = new KisPixelSelection();
    src->select(QRect(0, 0, 20, 20));
    QCOMPARE(src->selectedExactRect(), QRect(0, 0, 20, 20));

    KisPixelSelectionSP Selection = new KisPixelSelection();
    Selection->select(QRect(10, 10, 20, 20));
    QCOMPARE(Selection->selectedExactRect(), QRect(10, 10, 20, 20));
    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(Selection);
    sel->updateProjection();
    KisPixelSelectionSP dst = new KisPixelSelection();
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bitBlt(0, 0, src, 0, 0, 30, 30);
    painter.end();

    QCOMPARE(dst->selectedExactRect(), QRect(10, 10, 10, 10));

    KisRectConstIteratorSP it = dst->createRectConstIteratorNG(10, 10, 10, 10);
    do {
        // These are selections, so only one channel and it should
        // be totally selected
        QCOMPARE(it->oldRawData()[0], MAX_SELECTED);
    } while (it->nextPixel());
}

/*

Test with non-square selection

0,0               0,30
  +-----------+------+
  |    13,13  |      |
  |      x +--+      |
  |     +--+##|      |
  |     |#####|      |
  +-----+-----+      |
  |         20,20    |
  |                  |
  |                  |
  +------------------+
                  30,30
 */
void KisPainterTest::testSelectionBltSelectionIrregular()
{

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    KisPixelSelectionSP src = new KisPixelSelection();
    src->select(QRect(0, 0, 20, 20));
    QCOMPARE(src->selectedExactRect(), QRect(0, 0, 20, 20));

    KisPixelSelectionSP Selection = new KisPixelSelection();
    Selection->select(QRect(10, 15, 20, 15));
    Selection->select(QRect(15, 10, 15, 5));
    QCOMPARE(Selection->selectedExactRect(), QRect(10, 10, 20, 20));
    QCOMPARE(TestUtil::alphaDevicePixel(Selection, 13, 13), MIN_SELECTED);

    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(Selection);
    sel->updateProjection();

    KisPixelSelectionSP dst = new KisPixelSelection();
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.bitBlt(0, 0, src, 0, 0, 30, 30);
    painter.end();

    QCOMPARE(dst->selectedExactRect(), QRect(10, 10, 10, 10));
    QCOMPARE(TestUtil::alphaDevicePixel(dst, 13, 13), MIN_SELECTED);
}

void KisPainterTest::testSelectionBitBltFixedSelection()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KoColor c(Qt::red, cs);
    c.setOpacity(quint8(128));
    src->fill(0, 0, 20, 20, c.data());

    QCOMPARE(src->exactBounds(), QRect(0, 0, 20, 20));

    KisFixedPaintDeviceSP fixedSelection = new KisFixedPaintDevice(cs);
    fixedSelection->setRect(QRect(0, 0, 20, 20));
    fixedSelection->initialize();
    KoColor fill(Qt::black, cs);
    fixedSelection->fill(5, 5, 10, 10, fill.data());
    fixedSelection->convertTo(KoColorSpaceRegistry::instance()->alpha8());

    KisPainter painter(dst);

    painter.bitBltWithFixedSelection(0, 0, src, fixedSelection, 20, 20);
    painter.end();

    QCOMPARE(dst->exactBounds(), QRect(5, 5, 10, 10));
    /*
qDebug() << "canary1.5";
    dst->clear();
    painter.begin(dst);

    painter.bitBltWithFixedSelection(0, 0, src, fixedSelection, 10, 20);
    painter.end();
qDebug() << "canary2";
    QCOMPARE(dst->exactBounds(), QRect(5, 5, 5, 10));

    dst->clear();
    painter.begin(dst);

    painter.bitBltWithFixedSelection(0, 0, src, fixedSelection, 5, 5, 5, 5, 10, 20);
    painter.end();
qDebug() << "canary3";
    QCOMPARE(dst->exactBounds(), QRect(5, 5, 5, 10));

    dst->clear();
    painter.begin(dst);

    painter.bitBltWithFixedSelection(5, 5, src, fixedSelection, 10, 20);
    painter.end();
qDebug() << "canary4";
    QCOMPARE(dst->exactBounds(), QRect(10, 10, 5, 10));
    */
}

void KisPainterTest::testSelectionBitBltEraseCompositeOp()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KoColor c(Qt::red, cs);
    dst->fill(0, 0, 150, 150, c.data());

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KoColor c2(Qt::black, cs);
    src->fill(50, 50, 50, 50, c2.data());

    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect(25, 25, 100, 100));

    KisSelectionSP sel = new KisSelection();
    sel->setPixelSelection(selection);
    sel->updateProjection();

    const KoCompositeOp* op = cs->compositeOp(COMPOSITE_ERASE);
    KisPainter painter(dst);
    painter.setSelection(sel);
    painter.setCompositeOp(op);
    painter.bitBlt(0, 0, src, 0, 0, 150, 150);
    painter.end();

    //dst->convertToQImage(0).save("result.png");

    QRect erasedRect(50, 50, 50, 50);
    KisRectConstIteratorSP it = dst->createRectConstIteratorNG(0, 0, 150, 150);
    do {
        if(!erasedRect.contains(it->x(), it->y())) {
             QVERIFY(memcmp(it->oldRawData(), c.data(), cs->pixelSize()) == 0);
        }
    } while (it->nextPixel());

}

void KisPainterTest::testSimpleAlphaCopy()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    quint8 p = 128;
    src->fill(0, 0, 100, 100, &p);
    QVERIFY(src->exactBounds() == QRect(0, 0, 100, 100));
    KisPainter gc(dst);
    gc.setCompositeOp(KoColorSpaceRegistry::instance()->alpha8()->compositeOp(COMPOSITE_COPY));
    gc.bitBlt(QPoint(0, 0), src, src->exactBounds());
    gc.end();
    QCOMPARE(dst->exactBounds(), QRect(0, 0, 100, 100));

}

void KisPainterTest::checkPerformance()
{
    KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    KisPaintDeviceSP dst = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    quint8 p = 128;
    src->fill(0, 0, 10000, 5000, &p);
    KisSelectionSP sel = new KisSelection();
    sel->getOrCreatePixelSelection()->select(QRect(0, 0, 10000, 5000), 128);
    sel->updateProjection();

    QTime t;
    t.start();
    for (int i = 0; i < 10; ++i) {
        KisPainter gc(dst);
        gc.bitBlt(0, 0, src, 0, 0, 10000, 5000);
    }

    t.restart();
    for (int i = 0; i < 10; ++i) {
        KisPainter gc(dst, sel);
        gc.bitBlt(0, 0, src, 0, 0, 10000, 5000);
    }
}

void KisPainterTest::testBitBltOldData()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    quint8 defaultPixel = 0;
    quint8 p1 = 128;
    quint8 p2 = 129;
    quint8 p3 = 130;
    KoColor defaultColor(&defaultPixel, cs);
    KoColor color1(&p1, cs);
    KoColor color2(&p2, cs);
    KoColor color3(&p3, cs);
    QRect fillRect(0,0,5000,5000);

    src->fill(fillRect, color1);

    KisPainter srcGc(src);
    srcGc.beginTransaction("");
    src->fill(fillRect, color2);

    KisPainter dstGc(dst);
    dstGc.bitBltOldData(QPoint(), src, fillRect);

    QVERIFY(TestUtil::checkAlphaDeviceFilledWithPixel(dst, fillRect, p1));

    dstGc.end();
    srcGc.deleteTransaction();
}

void KisPainterTest::benchmarkBitBlt()
{
    quint8 p = 128;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KoColor color(&p, cs);
    QRect fillRect(0,0,5000,5000);

    src->fill(fillRect, color);

    QBENCHMARK {
        KisPainter gc(dst);
        gc.bitBlt(QPoint(), src, fillRect);
    }
}

void KisPainterTest::benchmarkBitBltOldData()
{
    quint8 p = 128;
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KoColor color(&p, cs);
    QRect fillRect(0,0,5000,5000);

    src->fill(fillRect, color);

    QBENCHMARK {
        KisPainter gc(dst);
        gc.bitBltOldData(QPoint(), src, fillRect);
    }
}

QTEST_KDEMAIN(KisPainterTest, NoGUI)
#include "kis_painter_test.moc"


