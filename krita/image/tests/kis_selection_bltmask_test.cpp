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
#include "kis_selection_bltmask_test.h"

#include <QRect>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pixel_selection.h"

void KisSelectionBltMaskTest::testSelectionBltMask()
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "temporary");

    KisPixelSelectionSP selection1 = KisPixelSelectionSP(new KisPixelSelection(dev));
    selection1->select(QRect(0,0,20,20));

    KisPixelSelectionSP selection2 = KisPixelSelectionSP(new KisPixelSelection(dev));
    selection1->select(QRect(10,10,20,20));

    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection(dev));
    KisPainter painter(tmpSel);
    QRect r = QRect(0,0,30,30);
    painter.bltMask(r.x(), r.y(),  selection1->colorSpace()->compositeOp(COMPOSITE_OVER), KisPaintDeviceSP(selection1.data()),
                    KisPaintDeviceSP(selection2.data()), OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    painter.end();

    QRect selectionExtent = tmpSel->selectedExactRect();
    QCOMPARE( selectionExtent.width(), 10 );
    QCOMPARE( selectionExtent.height(), 10 );
}

QTEST_KDEMAIN(KisSelectionBltMaskTest, NoGUI)
#include "kis_selection_bltmask_test.moc"


