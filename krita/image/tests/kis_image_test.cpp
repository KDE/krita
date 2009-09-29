/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_image_test.h"
#include <QApplication>

#include <qtest_kde.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>


#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include <kis_debug.h>

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

void KisImageTest::layerTests()
{
    KisImageWSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE);
    image->addNode(layer);

    QVERIFY(image->rootLayer()->firstChild()->objectName() == layer->objectName());
}

QTEST_KDEMAIN(KisImageTest, NoGUI)
#include "kis_image_test.moc"
