/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include <KoColorSpaceRegistry.h>
#include "kis_crop_visitor_test.h"

#include <qtest_kde.h>
#include "kis_crop_visitor.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include "kis_transparency_mask.h"

#include "testutil.h"
#include "kis_undo_stores.h"

void KisCropVisitorTest::testCreation()
{
    QRect rc(0, 0, 100, 100);
    KisCropVisitor test(rc, true);
}

void KisCropVisitorTest::testUndo()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisImageSP image = new KisImage(undoStore, 300, 300, cs, "test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "testlayer", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP dev = layer->paintDevice();

    KisFillPainter painter(dev);
    painter.fillRect(QRect(0, 0, 300, 300), KoColor(Qt::white, cs));
    QImage image1 = dev->convertToQImage(0, 0, 0, 300, 300);

    undoStore->beginMacro("");

    QRect rc(0, 0, 100, 100);
    KisCropVisitor visitor(rc, true);
    layer->accept(visitor);

    undoStore->endMacro();
    undoStore->undo();

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image1, dev->convertToQImage(0, 0, 0, 300, 300))) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisCropVisitorTest::testCropTransparencyMask()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisImageSP image = new KisImage(0, 300, 300, cs, "test", false);
    KisPaintLayerSP layer = new KisPaintLayer(image, "testlayer", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP dev = layer->paintDevice();

    QRect fillRect(0,0,300,300);
    dev->fill(fillRect, KoColor(Qt::white, cs));

    KisTransparencyMaskSP mask = new KisTransparencyMask();

    image->addNode(layer, image->rootLayer());
    image->addNode(mask, layer);

    QRect selectionRect(50,50,100,100);

    mask->setSelection(new KisSelection());
    KisPixelSelectionSP pixelSelection = mask->selection()->getOrCreatePixelSelection();
    pixelSelection->select(selectionRect, MAX_SELECTED);

    QCOMPARE(pixelSelection->selectedExactRect(), selectionRect);

    QRect cropRect(75,75,50,50);

    KisCropVisitor visitor(cropRect, false);

    mask->accept(visitor);

    QCOMPARE(pixelSelection->selectedExactRect(), cropRect);
}


QTEST_KDEMAIN(KisCropVisitorTest, GUI)
#include "kis_crop_visitor_test.moc"
