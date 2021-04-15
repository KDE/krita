/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_mask_test.h"

#include <simpletest.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_image.h"
#include "kis_selection_mask.h"
#include "kis_group_layer.h"


void KisSelectionMaskTest::testActivation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 100, 100, cs, "bla");
    KisSelectionMaskSP mask1 = new KisSelectionMask(image);
    KisSelectionMaskSP mask2 = new KisSelectionMask(image);
    KisSelectionMaskSP mask3 = new KisSelectionMask(image);

    image->addNode(mask1);
    image->addNode(mask2);
    image->addNode(mask3);

    QCOMPARE(mask1->active(), false);
    QCOMPARE(mask2->active(), false);
    QCOMPARE(mask3->active(), false);
    QCOMPARE(image->rootLayer()->selectionMask(), KisSelectionMaskSP(0));

    mask1->setActive(true);

    QCOMPARE(mask1->active(), true);
    QCOMPARE(mask2->active(), false);
    QCOMPARE(mask3->active(), false);
    QCOMPARE(image->rootLayer()->selectionMask(), mask1);

    mask2->setActive(true);

    QCOMPARE(mask1->active(), false);
    QCOMPARE(mask2->active(), true);
    QCOMPARE(mask3->active(), false);
    QCOMPARE(image->rootLayer()->selectionMask(), mask2);

    mask3->setActive(true);

    QCOMPARE(mask1->active(), false);
    QCOMPARE(mask2->active(), false);
    QCOMPARE(mask3->active(), true);
    QCOMPARE(image->rootLayer()->selectionMask(), mask3);

    mask3->setActive(false);

    QCOMPARE(mask1->active(), false);
    QCOMPARE(mask2->active(), false);
    QCOMPARE(mask3->active(), false);
    QCOMPARE(image->rootLayer()->selectionMask(), KisSelectionMaskSP(0));

}


SIMPLE_TEST_MAIN(KisSelectionMaskTest)
