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

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"
#include <kis_debug.h>
#include <kis_layer_composition.h>

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

void KisImageTest::layerTests()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    QVERIFY(image->rootLayer()->firstChild()->objectName() == layer->objectName());
}

void KisImageTest::testConvertImageColorSpace()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    KisPaintDeviceSP device1 = new KisPaintDevice(cs8);
    KisLayerSP paint1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfiguration *configuration = filter->defaultConfiguration(0);
    Q_ASSERT(configuration);

    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);

    image->addNode(paint1, image->root());
    image->addNode(blur1, image->root());

    image->refreshGraph();

    const KoColorSpace *cs16 = KoColorSpaceRegistry::instance()->rgb16();
    image->lock();
    image->convertImageColorSpace(cs16,
                                  KoColorConversionTransformation::IntentPerceptual,
                                  KoColorConversionTransformation::Empty);
    image->unlock();

    QVERIFY(*cs16 == *image->colorSpace());
    QVERIFY(*cs16 == *image->root()->colorSpace());
    QVERIFY(*cs16 == *paint1->colorSpace());
    QVERIFY(*cs16 == *blur1->colorSpace());

    QVERIFY(!image->root()->compositeOp());
    QVERIFY(*cs16 == *paint1->compositeOp()->colorSpace());
    QVERIFY(*cs16 == *blur1->compositeOp()->colorSpace());

    image->refreshGraph();
}

void KisImageTest::testGlobalSelection()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 0U);

    KisSelectionSP selection1 = new KisSelection(new KisDefaultBounds(image));
    KisSelectionSP selection2 = new KisSelection(new KisDefaultBounds(image));

    image->setGlobalSelection(selection1);
    QCOMPARE(image->globalSelection(), selection1);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->setGlobalSelection(selection2);
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->deselectGlobalSelection();
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->reselectGlobalSelection();
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    // mixed deselecting/setting/reselecting

    image->deselectGlobalSelection();
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->setGlobalSelection(selection1);
    QCOMPARE(image->globalSelection(), selection1);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);
}

void KisImageTest::testLayerComposition()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    image->addNode(layer);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer 2", OPACITY_OPAQUE_U8);
    image->addNode(layer2);

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    KisLayerComposition comp(image, "comp 1");
    comp.store();

    layer2->setVisible(false);

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());

    KisLayerComposition comp2(image, "comp 2");
    comp2.store();

    comp.apply();

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    comp2.apply();

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());
}


QTEST_KDEMAIN(KisImageTest, NoGUI)
#include "kis_image_test.moc"
