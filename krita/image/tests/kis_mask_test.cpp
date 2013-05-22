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

#include "kis_mask_test.h"

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_node.h"
#include "kis_mask.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "testutil.h"


class TestMask : public KisMask
{
public:
    using KisMask::apply;

    TestMask() : KisMask("TestMask") {
    }

    KisNodeSP clone() const {
        return new TestMask(*this);
    }

    bool allowAsChild(KisNodeSP) const {
        return false;
    }

};


void KisMaskTest::testCreation()
{
    TestUtil::MaskParent p;
    TestMask mask;
    mask.initSelection(p.layer);

    QCOMPARE(mask.extent(), QRect(0,0,512,512));
    QCOMPARE(mask.exactBounds(), QRect(0,0,512,512));
}

void KisMaskTest::testSelection()
{
    TestUtil::MaskParent p;
    TestMask mask;

    KisSelectionSP sel = new KisSelection();
    sel->getOrCreatePixelSelection()->select(QRect(0,0,100,100), MAX_SELECTED);

    mask.initSelection(sel, p.layer);

    QCOMPARE(mask.extent(), QRect(0,0,128,128));
    QCOMPARE(mask.exactBounds(), QRect(0,0,100,100));

    mask.select(QRect(0,0,500,500), MAX_SELECTED);;

    QCOMPARE(mask.extent(), QRect(0,0,512,512));
    QCOMPARE(mask.exactBounds(), QRect(0,0,500,500));
}

void KisMaskTest::testCropUpdateBySelection()
{
    TestUtil::MaskParent p;

    /**
     * We do not use exact selection bounds for cropping,
     * so the rects should be covered by different tiles
     */
    QRect selectionRect(10, 10, 20, 20);
    QRect updateRect(64, 64, 20, 20);

    TestMask mask;

    KisSelectionSP sel = new KisSelection();
    sel->getOrCreatePixelSelection()->select(selectionRect, MAX_SELECTED);

    mask.initSelection(sel, p.layer);

    mask.apply(p.layer->projection(), updateRect);
    // Here we crash! :)

    /**
     * If you see a crash, it means KisMask tried to update
     * the area that is outside its selection.
     * Please consider fixing KisMask::apply() first
     */
}

void KisMaskTest::testSelectionParent()
{
    {
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(0, 100, 100, cs, "stest");

        KisMaskSP mask = new TestMask;
        mask->initSelection(image->rootLayer());
        KisSelectionSP selection = mask->selection();
        QCOMPARE(selection->parentNode(), KisNodeWSP(mask));
    }

    {
        KisMaskSP mask = new TestMask;
        mask->setSelection(new KisSelection());
        KisSelectionSP selection = mask->selection();
        QCOMPARE(selection->parentNode(), KisNodeWSP(mask));
    }
}

QTEST_KDEMAIN(KisMaskTest, GUI)
#include "kis_mask_test.moc"
