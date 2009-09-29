/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_projection_test.h"
#include <qtest_kde.h>


#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_fill_painter.h"

void KisProjectionTest::testDirty()
{
    KisImageWSP image = new KisImage(0, 1000, 1000, 0, "layer tests");

    // Two layers so the single-layer-is-rootlayer optimization doesn't kick in
    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer 2", OPACITY_OPAQUE);
    image->addNode(layer);
    image->addNode(layer2);
    KisFillPainter gc(layer2->paintDevice());
    KoColor c(Qt::red, layer2->colorSpace());
    gc.fillRect(0, 0, 1000, 1000, c);
    gc.end();
    layer2->setDirty(gc.dirtyRegion());

    // wait a little for the projection to finish
    QTest::qSleep(250);

    // Check that the projection is totally redistribute
    KisRectConstIteratorPixel iter = image->projection()->createRectConstIterator(0, 0, 1000, 1000);
    while (!iter.isDone()) {
        QColor c;
        image->colorSpace()->toQColor(iter.rawData(), &c, image->profile());
        QVERIFY(c == Qt::red);
        ++iter;
    }
}

QTEST_KDEMAIN(KisProjectionTest, NoGUI)
#include "kis_projection_test.moc"


