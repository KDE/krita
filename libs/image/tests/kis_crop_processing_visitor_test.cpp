/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_crop_processing_visitor_test.h"

#include <QTest>
#include <KoColorSpaceRegistry.h>
#include "processing/kis_crop_processing_visitor.h"
#include "commands_new/kis_processing_command.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include "kis_transparency_mask.h"
#include <testutil.h>
#include "kis_surrogate_undo_adapter.h"
#include "kis_image.h"
#include "kis_pixel_selection.h"

void KisCropProcessingVisitorTest::testUndo()
{
    KisSurrogateUndoAdapter undoAdapter;

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 300, 300, cs, "test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "testlayer", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP device = layer->paintDevice();

    QRect fillRect(50,50,100,100);
    QRect cropRect(25,25,100,100);

    KisFillPainter painter(device);
    painter.fillRect(fillRect, KoColor(Qt::white, cs));
    QImage image1 = device->convertToQImage(0, 0, 0, 300, 300);

    undoAdapter.beginMacro(kundo2_noi18n("test"));

    KisCropProcessingVisitor visitor(cropRect, true, true);
    layer->accept(visitor, &undoAdapter);

    undoAdapter.endMacro();
    undoAdapter.undo();

    QImage image2 = device->convertToQImage(0, 0, 0, 300, 300);

    QCOMPARE(image1, image2);
}

void KisCropProcessingVisitorTest::testCropTransparencyMask()
{
    KisSurrogateUndoAdapter undoAdapter;

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 300, 300, cs, "test");

    QRect cropRect(25,25,100,100);
    QRect selectionRect(40,40,100,100);
    KisTransparencyMaskSP mask = new KisTransparencyMask(image, "tmask");
    mask->testingInitSelection(selectionRect, 0);
    KisPixelSelectionSP pixelSelection = mask->selection()->pixelSelection();

    QCOMPARE(pixelSelection->selectedExactRect(), selectionRect);

    undoAdapter.beginMacro(kundo2_noi18n("test"));
    KisCropProcessingVisitor visitor(cropRect, true, true);
    mask->accept(visitor, &undoAdapter);
    undoAdapter.endMacro();

    QCOMPARE(pixelSelection->selectedExactRect(), QRect(15,15,85,85));

    undoAdapter.undo();

    QCOMPARE(pixelSelection->selectedExactRect(), selectionRect);

}

void KisCropProcessingVisitorTest::testWrappedInCommand()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 300, 300, cs, "test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "testlayer", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP device = layer->paintDevice();

    QRect fillRect(50,50,100,100);
    QRect cropRect(25,25,100,100);

    KisFillPainter painter(device);
    painter.fillRect(fillRect, KoColor(Qt::white, cs));
    QImage image1 = device->convertToQImage(0, 0, 0, 300, 300);


    KisProcessingVisitorSP visitor = new KisCropProcessingVisitor(cropRect, true, true);

    KisProcessingCommand command(visitor, layer);

    command.redo();

    QImage image2 = device->convertToQImage(0, 0, 0, 300, 300);

    command.undo();

    QImage image3 = device->convertToQImage(0, 0, 0, 300, 300);

    command.redo();

    QImage image4 = device->convertToQImage(0, 0, 0, 300, 300);

    command.undo();

    QImage image5 = device->convertToQImage(0, 0, 0, 300, 300);

    QCOMPARE(image2, image4);
    QCOMPARE(image1, image3);
    QCOMPARE(image1, image5);
}

QTEST_MAIN(KisCropProcessingVisitorTest)
