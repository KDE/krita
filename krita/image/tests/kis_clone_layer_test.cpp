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

#include "kis_clone_layer_test.h"

#include <qtest_kde.h>


#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_image.h"

void KisCloneLayerTest::testCreation()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    KisLayerSP layer = new KisPaintLayer(image, "clone test", OPACITY_OPAQUE);

    KisCloneLayer test(layer, image, "clonetest", OPACITY_OPAQUE
                      );
}


QTEST_KDEMAIN(KisCloneLayerTest, GUI)
#include "kis_clone_layer_test.moc"
