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

#include <qtest_kde.h>
#include "kis_pixel_selection_test.h"

#include <kdebug.h>
#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_transparency_mask.h"
#include "kis_pixel_selection.h"

void KisPixelSelectionTest::testCreation()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer( image, "test", OPACITY_OPAQUE );
    KisPaintDeviceSP dev = layer->paintDevice();
    KisTransparencyMaskSP mask = new KisTransparencyMask();

    KisPixelSelectionSP selection = new KisPixelSelection();
    QVERIFY( selection );
    QVERIFY( selection->isTotallyUnselected(QRect( 0, 0, 512, 512 )) );
    QVERIFY( selection->interestedInDirtyness() == false );

    selection = new KisPixelSelection( dev );
    QVERIFY( selection );
    QVERIFY( selection->isTotallyUnselected(QRect( 0, 0, 512, 512 )) );
    QVERIFY( selection->interestedInDirtyness() == false );
    selection->setInterestedInDirtyness( true );
    selection->setDirty( QRect( 10, 10, 10, 10 ) );


    selection = new KisPixelSelection( dev, mask.data() );

}

void KisPixelSelectionTest::testInvert()
{
    KisPixelSelectionSP selection = new KisPixelSelection();
    selection->select(QRect( 5, 5, 10, 10));
    selection->invert();
    QCOMPARE( selection->selected( 20, 20), MAX_SELECTED);
}

QTEST_KDEMAIN(KisPixelSelectionTest, NoGUI)
#include "kis_pixel_selection_test.moc"


