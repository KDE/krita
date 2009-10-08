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

#include "kis_colorspace_convert_visitor_test.h"

#include <qtest_kde.h>
#include <KoColorTransformation.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <testutil.h>

#include "kis_colorspace_convert_visitor.h"
#include "kis_paint_layer.h"
#include "kis_image.h"

void KisColorSpaceConvertVisitorTest::testCreation()
{
    const KoColorSpace * rgb = KoColorSpaceRegistry::instance()->rgb16();
    QVERIFY(rgb);
    const KoColorSpace * lab = KoColorSpaceRegistry::instance()->lab16();
    QVERIFY(lab);
    TestUtil::KisUndoAdapterDummy* undoAdapterDummy = new TestUtil::KisUndoAdapterDummy();
    KisImageWSP img = new KisImage(undoAdapterDummy, 100, 100, lab, "test");
    KisPaintLayerSP layer = new KisPaintLayer(img, "test", OPACITY_OPAQUE, lab);
    KisColorSpaceConvertVisitor test(img, rgb, KoColorConversionTransformation::IntentPerceptual);
    layer->accept(test);
    QVERIFY(layer->colorSpace()->colorModelId() == rgb->colorModelId());
}


QTEST_KDEMAIN(KisColorSpaceConvertVisitorTest, GUI)
#include "kis_colorspace_convert_visitor_test.moc"
