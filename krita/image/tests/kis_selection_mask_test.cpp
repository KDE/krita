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

#include "kis_selection_mask_test.h"

#include <qtest_kde.h>

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


QTEST_KDEMAIN(KisSelectionMaskTest, GUI)
#include "kis_selection_mask_test.moc"
