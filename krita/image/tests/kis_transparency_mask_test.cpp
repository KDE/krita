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

#include "kis_transparency_mask_test.h"

#include <qtest_kde.h>
#include "kis_transparency_mask.h"
#include "kis_fill_painter.h"
#include "testutil.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"

KisPaintDeviceSP createDevice()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs, "bla");
    KisFillPainter gc(dev);
    KoColor c(Qt::red, dev->colorSpace());
    gc.fillRect(0, 0, 100, 100, c);
    c = KoColor(Qt::blue, dev->colorSpace());
    gc.fillRect(100, 0, 100, 100, c);
    gc.end();

    return dev;
}

void KisTransparencyMaskTest::testCreation()
{
    KisTransparencyMask test;
}

void KisTransparencyMaskTest::testApply()
{
    QPoint errpoint;
    
    KisTransparencyMaskSP mask = new KisTransparencyMask();
    
    // Nothing is selected -- therefore everything should be filtered out on apply
    KisPaintDeviceSP dev = createDevice();
    mask->apply(dev, QRect(0, 0, 200, 100));
    QImage qimg = dev->convertToQImage(0, 0, 0, 200, 100);

    if ( !TestUtil::compareQImages( errpoint,
            QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_1.png"),
            qimg ) ) {
        QFAIL( QString( "Failed to mask out image, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
    
    // Invert the mask -- everything is selected
    dev = createDevice();
    mask->selection()->getOrCreatePixelSelection()->invert();
    mask->apply(dev, QRect(0, 0, 200, 100));
    qimg = dev->convertToQImage(0, 0, 0, 200, 100);

    if ( !TestUtil::compareQImages( errpoint,
            QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_2.png"),
            qimg ) ) {
        QFAIL( QString( "Failed to mask in image, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
    
    // Invert back, and select a small area
    dev = createDevice();
    mask->selection()->getOrCreatePixelSelection()->invert();
    mask->select(QRect(50, 0, 100, 100));
    mask->apply(dev, QRect(0, 0, 200, 100));
    qimg = dev->convertToQImage(0, 0, 0, 200, 100);

    if ( !TestUtil::compareQImages( errpoint,
            QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_3.png"),
            qimg ) ) {

        QFAIL( QString( "Failed to apply partial mask, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
    
}

QTEST_KDEMAIN(KisTransparencyMaskTest, GUI)
#include "kis_transparency_mask_test.moc"
