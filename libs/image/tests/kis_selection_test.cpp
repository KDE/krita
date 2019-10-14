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

#include "kis_selection_test.h"
#include <QTest>


#include <kis_debug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>

#include "kis_datamanager.h"
#include "kis_pixel_selection.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_mask.h"
#include "kis_image.h"
#include "kis_transparency_mask.h"
#include "testutil.h"

#include <KoColorModelStandardIds.h>

void KisSelectionTest::testGrayColorspaceConversion()
{
    const KoColorSpace *csA =
        KoColorSpaceRegistry::instance()->
        colorSpace(GrayAColorModelID.id(),
                   Integer8BitsColorDepthID.id(),
                   QString());

    const KoColorSpace *csNoA =
        KoColorSpaceRegistry::instance()->alpha8();

    QVERIFY(csA);
    QVERIFY(csNoA);

    QCOMPARE(csA->pixelSize(), 2U);
    QCOMPARE(csNoA->pixelSize(), 1U);

    quint8 color1[1] = {128};
    quint8 color2[2] = {64,32};

    csA->convertPixelsTo(color2, color1, csNoA, 1,
                         KoColorConversionTransformation::internalRenderingIntent(),
                         KoColorConversionTransformation::internalConversionFlags());

    QCOMPARE((int)color1[0], 8);

    csNoA->convertPixelsTo(color1, color2, csA, 1,
                           KoColorConversionTransformation::internalRenderingIntent(),
                           KoColorConversionTransformation::internalConversionFlags());

    QCOMPARE((int)color2[0], 8);
    QCOMPARE((int)color2[1], 255);
}

void KisSelectionTest::testGrayColorspaceOverComposition()
{
    const KoColorSpace *csA =
        KoColorSpaceRegistry::instance()->
        colorSpace(GrayAColorModelID.id(),
                   Integer8BitsColorDepthID.id(),
                   QString());
    const KoColorSpace *csNoA =
        KoColorSpaceRegistry::instance()->alpha8();

    QVERIFY(csA);
    QVERIFY(csNoA);

    QCOMPARE(csA->pixelSize(), 2U);
    QCOMPARE(csNoA->pixelSize(), 1U);

    quint8 color0[2] = {32,255};
    quint8 color1[2] = {128,64};
    quint8 color3[1] = {32};

    KoCompositeOp::ParameterInfo params;
    params.dstRowStart = color0;
    params.dstRowStride = 0;
    params.srcRowStart = color1;
    params.srcRowStride = 0;
    params.maskRowStart = 0;
    params.maskRowStride = 0;
    params.rows = 1;
    params.cols = 1;
    params.opacity = 1.0;
    params.flow = 1.0;

    csA->bitBlt(csA, params, csA->compositeOp(COMPOSITE_OVER),
                KoColorConversionTransformation::internalRenderingIntent(),
                KoColorConversionTransformation::internalConversionFlags());

    QCOMPARE((int)color0[0], 56);
    QCOMPARE((int)color0[1], 255);

    params.dstRowStart = color3;

    csNoA->bitBlt(csA, params, csNoA->compositeOp(COMPOSITE_OVER),
                  KoColorConversionTransformation::internalRenderingIntent(),
                  KoColorConversionTransformation::internalConversionFlags());

    QCOMPARE((int)color3[0], 56);
}

void KisSelectionTest::testSelectionComponents()
{
    KisSelectionSP selection = new KisSelection();

    QCOMPARE(selection->hasPixelSelection(), false);
    QCOMPARE(selection->hasShapeSelection(), false);
    QCOMPARE(selection->shapeSelection(), (void*)0);

    selection->pixelSelection()->select(QRect(10,10,10,10));
    QCOMPARE(selection->hasPixelSelection(), true);
    QCOMPARE(selection->selectedExactRect(), QRect(10,10,10,10));
}

void KisSelectionTest::testSelectionActions()
{
    KisSelectionSP selection = new KisSelection();
    QVERIFY(selection->hasPixelSelection() == false);
    QVERIFY(selection->hasShapeSelection() == false);

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(0, 0, 20, 20));

    KisPixelSelectionSP tmpSel = new KisPixelSelection();
    tmpSel->select(QRect(10, 0, 20, 20));

    pixelSelection->applySelection(tmpSel, SELECTION_ADD);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(0, 0, 30, 20));
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 30, 20));

    pixelSelection->clear();
    pixelSelection->select(QRect(0, 0, 20, 20));

    pixelSelection->applySelection(tmpSel, SELECTION_SUBTRACT);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(0, 0, 10, 20));
    QCOMPARE(selection->selectedExactRect(), QRect(0, 0, 10, 20));

    pixelSelection->clear();
    pixelSelection->select(QRect(0, 0, 20, 20));

    pixelSelection->applySelection(tmpSel, SELECTION_INTERSECT);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(10, 0, 10, 20));
    QCOMPARE(selection->selectedExactRect(), QRect(10, 0, 10, 20));
}

void KisSelectionTest::testInvertSelection()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1024, 1024, cs, "stest");

    KisSelectionSP selection = new KisSelection(new KisDefaultBounds(image));
    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(QRect(20, 20, 20, 20));

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 30, 30), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 0, 0), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 512, 512), MIN_SELECTED);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(20, 20, 20, 20));

    pixelSelection->invert();

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 100, 100), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 22, 22), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 0, 0), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 512, 512), MAX_SELECTED);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(0,0,1024,1024));
    QCOMPARE(pixelSelection->selectedRect(), QRect(0,0,1024,1024));

    selection->updateProjection();

    QCOMPARE(selection->selectedExactRect(), QRect(0,0,1024,1024));
    QCOMPARE(selection->selectedRect(), QRect(0,0,1024,1024));

    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 100, 100), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 22, 22), MIN_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 10, 10), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 0, 0), MAX_SELECTED);
    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 512, 512), MAX_SELECTED);
}

void KisSelectionTest::testInvertSelectionSemi()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1024, 1024, cs, "stest");

    KisSelectionSP selection = new KisSelection(new KisDefaultBounds(image));
    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    quint8 selectedness = 42;
    pixelSelection->select(QRect(20, 20, 20, 20), selectedness);

    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 30, 30), selectedness);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 0, 0), MIN_SELECTED);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(20, 20, 20, 20));

    pixelSelection->invert();

    quint8 invertedSelectedness = MAX_SELECTED - selectedness;
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 30, 30), invertedSelectedness);
    QCOMPARE(TestUtil::alphaDevicePixel(pixelSelection, 0, 0), MAX_SELECTED);
    QCOMPARE(pixelSelection->selectedExactRect(), QRect(0,0,1024,1024));
    QCOMPARE(pixelSelection->selectedRect(), QRect(0,0,1024,1024));

    selection->updateProjection();

    QCOMPARE(selection->selectedExactRect(), QRect(0,0,1024,1024));
    QCOMPARE(selection->selectedRect(), QRect(0,0,1024,1024));

    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 30, 30), invertedSelectedness);
    QCOMPARE(TestUtil::alphaDevicePixel(selection->projection(), 0, 0), MAX_SELECTED);
}

void KisSelectionTest::testCopy()
{
    KisSelectionSP sel = new KisSelection();
    sel->pixelSelection()->select(QRect(10, 10, 200, 200), 128);

    sel->updateProjection();

    KisSelectionSP sel2 = new KisSelection(*sel.data());
    QCOMPARE(sel2->selectedExactRect(), sel->selectedExactRect());

    QPoint errpoint;
    if (!TestUtil::comparePaintDevices(errpoint, sel->projection(), sel2->projection())) {
        sel2->projection()->convertToQImage(0, 0, 0, 200, 200).save("merge_visitor6.png");
        QFAIL(QString("Failed to copy selection, first different pixel: %1,%2 ")
              .arg(errpoint.x())
              .arg(errpoint.y())
              .toLatin1());
    }
}

void KisSelectionTest::testSelectionExactBounds()
{
    QRect referenceImageRect(0,0,1000,1000);
    QRect referenceDeviceRect(100,100,1040,1040);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisImageSP image = new KisImage(0, referenceImageRect.width(),
                                    referenceImageRect.height(),
                                    cs, "stest");

    KisPaintDeviceSP device = new KisPaintDevice(cs);
    device->fill(referenceDeviceRect, KoColor(Qt::white, cs));

    QCOMPARE(device->exactBounds(), referenceDeviceRect);

    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(device));

    quint8 defaultPixel = MAX_SELECTED;
    selection->pixelSelection()->setDefaultPixel(KoColor(&defaultPixel, selection->pixelSelection()->colorSpace()));

    // the selection uses device's extent only for performance reasons
    // \see bug 320213
    QCOMPARE(selection->selectedExactRect(), device->extent() | referenceImageRect);
}

void KisSelectionTest::testSetParentNodeAfterCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 100, 100, cs, "stest");
    KisSelectionSP selection = new KisSelection();
    KisPixelSelectionSP pixelSelection = selection->pixelSelection();

    QCOMPARE(selection->parentNode(), KisNodeWSP(0));
    QCOMPARE(selection->pixelSelection()->parentNode(), KisNodeWSP(0));

    selection->setParentNode(image->root());

    QCOMPARE(selection->parentNode(), KisNodeWSP(image->root()));
    QCOMPARE(selection->pixelSelection()->parentNode(), KisNodeWSP(image->root()));
}

void KisSelectionTest::testSetParentNodeBeforeCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 100, 100, cs, "stest");
    KisSelectionSP selection = new KisSelection();

    selection->setParentNode(image->root());

    KisPixelSelectionSP pixelSelection = selection->pixelSelection();

    QCOMPARE(selection->parentNode(), KisNodeWSP(image->root()));
    QCOMPARE(selection->pixelSelection()->parentNode(), KisNodeWSP(image->root()));
}

void KisSelectionTest::testOutlineGeneration()
{
    KisSelectionSP sel = new KisSelection();
    sel->pixelSelection()->select(QRect(428,436, 430,211), 128);

    QVERIFY(sel->outlineCacheValid());

    QPainterPath originalOutline = sel->outlineCache();

    sel->pixelSelection()->invalidateOutlineCache();
    sel->recalculateOutlineCache();

    QPainterPath calculatedOutline = sel->outlineCache();

    QPainterPath closedSubPath = calculatedOutline;
    closedSubPath.closeSubpath();

    /**
     * Our outline generation code has a small problem: it can
     * generate a polygon, which isn't closed (it'll repeat the first
     * point instead). There is a special workaround for it in
     * KisPixelSelection::recalculateOutlineCache(), which explicitly
     * closes the path, so here we just check it.
     */

    bool isClosed = closedSubPath == calculatedOutline;
    QVERIFY(isClosed);
}

KISTEST_MAIN(KisSelectionTest)


