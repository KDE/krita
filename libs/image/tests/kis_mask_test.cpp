/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mask_test.h"

#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_node.h"
#include "kis_mask.h"
#include "kis_selection.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include <testutil.h>


class TestMask : public KisMask
{
public:
    using KisMask::apply;

    TestMask(KisImageWSP image) : KisMask(image, "TestMask") {
    }

    KisNodeSP clone() const override {
        return new TestMask(*this);
    }

    bool allowAsChild(KisNodeSP) const override {
        return false;
    }

};

typedef KisSharedPtr<TestMask> TestMaskSP;


void KisMaskTest::testCreation()
{
    TestUtil::MaskParent p;
    TestMaskSP mask = new TestMask(p.image);
    mask->initSelection(p.layer);

    QCOMPARE(mask->extent(), QRect(0,0,512,512));
    QCOMPARE(mask->exactBounds(), QRect(0,0,512,512));
}

void KisMaskTest::testSelection()
{
    TestUtil::MaskParent p;
    TestMaskSP mask = new TestMask(p.image);

    KisSelectionSP sel = new KisSelection();
    sel->pixelSelection()->select(QRect(0,0,100,100), MAX_SELECTED);

    mask->initSelection(sel, p.layer);

    QCOMPARE(mask->extent(), QRect(0,0,128,128));
    QCOMPARE(mask->exactBounds(), QRect(0,0,100,100));

    mask->select(QRect(0,0,500,500), MAX_SELECTED);

    QCOMPARE(mask->extent(), QRect(0,0,512,512));
    QCOMPARE(mask->exactBounds(), QRect(0,0,500,500));
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

    TestMaskSP mask = new TestMask(p.image);

    KisSelectionSP sel = new KisSelection();
    sel->pixelSelection()->select(selectionRect, MAX_SELECTED);

    mask->initSelection(sel, p.layer);

    mask->apply(p.layer->projection(), updateRect, updateRect, KisNode::N_FILTHY);
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

        KisMaskSP mask = new TestMask(image);
        mask->initSelection(image->rootLayer());
        KisSelectionSP selection = mask->selection();
        QCOMPARE(selection->parentNode(), KisNodeWSP(mask));
    }

    {
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(0, 100, 100, cs, "stest");

        KisMaskSP mask = new TestMask(image);
        mask->setSelection(new KisSelection());
        KisSelectionSP selection = mask->selection();
        QCOMPARE(selection->parentNode(), KisNodeWSP(mask));
    }
}

void KisMaskTest::testDeferredOffsetInitialization()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 100, 100, cs, "stest");

    KisMaskSP mask = new TestMask(image);

    QCOMPARE(mask->x(), 0);
    QCOMPARE(mask->y(), 0);

    mask->setX(10);
    QCOMPARE(mask->x(), 10);
    QCOMPARE(mask->y(), 0);

    mask->setY(11);
    QCOMPARE(mask->x(), 10);
    QCOMPARE(mask->y(), 11);

    mask->initSelection(image->rootLayer());

    // IMPORTANT: a bit weird behavior, but it is needed for
    // KisKraLoadVisitor to work properly
    QCOMPARE(mask->x(), 10);
    QCOMPARE(mask->y(), 11);

    // Now there is no deferred initialization, so the offset
    // should simply be reset
    mask->initSelection(image->rootLayer());
    QCOMPARE(mask->x(), 0);
    QCOMPARE(mask->y(), 0);

    KisSelectionSP selection = mask->selection();
    QCOMPARE(selection->parentNode(), KisNodeWSP(mask));
}

SIMPLE_TEST_MAIN(KisMaskTest)
