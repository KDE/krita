/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_adjustment_layer_test.h"

#include <QTest>


#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_adjustment_layer.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_types.h"
#include "kis_datamanager.h"
#include "kis_pixel_selection.h"
#include <testutil.h>
#include <KisGlobalResourcesInterface.h>

void KisAdjustmentLayerTest::testCreation()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "adj layer test");
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisAdjustmentLayerSP test = new KisAdjustmentLayer(image, "test", kfc->cloneWithResourcesSnapshot(), 0);
}

void KisAdjustmentLayerTest::testSetSelection()
{
    KisSelectionSP sel = new KisSelection();
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "adj layer test");
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);
    sel->pixelSelection()->select(QRect(10, 10, 200, 200), 128);
    KisAdjustmentLayerSP l1 = new KisAdjustmentLayer(image, "bla", kfc->cloneWithResourcesSnapshot(), sel);
    QCOMPARE(sel->selectedExactRect(), l1->internalSelection()->selectedExactRect());
}

void KisAdjustmentLayerTest::testInverted()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "adj layer test");
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisSelectionSP sel2 = new KisSelection();
    sel2->pixelSelection()->invert();
    KisAdjustmentLayerSP l2 = new KisAdjustmentLayer(image, "bla", kfc->cloneWithResourcesSnapshot(), sel2);
    QCOMPARE(l2->internalSelection()->selectedExactRect(), image->bounds());

    KisSelectionSP sel3 = new KisSelection();
    sel3->pixelSelection()->select(QRect(50, -10, 800, 30), 128);
    l2->setInternalSelection(sel3);

}

void KisAdjustmentLayerTest::testSelectionParent()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "adj layer test");
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    {
        KisAdjustmentLayerSP adjLayer =
            new KisAdjustmentLayer(image, "bla", f->defaultConfiguration(KisGlobalResourcesInterface::instance())->cloneWithResourcesSnapshot(), 0);

        QCOMPARE(adjLayer->internalSelection()->parentNode(), KisNodeWSP(adjLayer));
    }

    {
        KisSelectionSP selection = new KisSelection();
        KisAdjustmentLayerSP adjLayer =
            new KisAdjustmentLayer(image, "bla", f->defaultConfiguration(KisGlobalResourcesInterface::instance())->cloneWithResourcesSnapshot(), selection);

        QCOMPARE(adjLayer->internalSelection()->parentNode(), KisNodeWSP(adjLayer));
    }

    {
        KisAdjustmentLayerSP adjLayer =
            new KisAdjustmentLayer(image, "bla", f->defaultConfiguration(KisGlobalResourcesInterface::instance())->cloneWithResourcesSnapshot(), 0);

        KisSelectionSP selection = new KisSelection();
        adjLayer->setInternalSelection(selection);

        QCOMPARE(adjLayer->internalSelection()->parentNode(), KisNodeWSP(adjLayer));
    }
}

QTEST_MAIN(KisAdjustmentLayerTest)
