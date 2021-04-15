/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_juggler_compressed_test.h"

#include <simpletest.h>
#include "kis_node_juggler_compressed.h"

#include <KoColor.h>
#include <KoColorSpace.h>

void KisNodeJugglerCompressedTest::init()
{
    p.reset(new TestUtil::MaskParent);

    QRect rect1(100, 100, 100, 100);
    QRect rect2(150, 150, 150, 150);

    layer1 = p->layer;
    layer1->paintDevice()->fill(rect1, KoColor(Qt::red, layer1->colorSpace()));

    layer2 = new KisPaintLayer(p->image, "paint2", OPACITY_OPAQUE_U8);
    layer2->paintDevice()->fill(rect2, KoColor(Qt::blue, layer2->colorSpace()));

    layer3 = new KisPaintLayer(p->image, "paint3", OPACITY_OPAQUE_U8);
    group4 = new KisGroupLayer(p->image, "group4", OPACITY_OPAQUE_U8);
    layer5 = new KisPaintLayer(p->image, "paint5", OPACITY_OPAQUE_U8);
    layer6 = new KisPaintLayer(p->image, "paint6", OPACITY_OPAQUE_U8);

    p->image->addNode(layer2);
    p->image->addNode(layer3);
    p->image->addNode(group4);
    p->image->addNode(layer5, group4);
    p->image->addNode(layer6);

    p->image->initialRefreshGraph();
}

void KisNodeJugglerCompressedTest::cleanup()
{
    p.reset();
    layer1.clear();
    layer2.clear();
}

void KisNodeJugglerCompressedTest::testMove(int delayBeforeEnd)
{
    TestUtil::ReferenceImageChecker chk("node_juggler", "move_test");
    chk.setMaxFailingPixels(0);

    KisNodeJugglerCompressed juggler(kundo2_i18n("Move Layer"), p->image, 0, 600);
    QVERIFY(chk.checkImage(p->image, "initial"));

    juggler.moveNode(layer1, p->image->root(), layer2);
    QTest::qWait(100);
    QVERIFY(chk.checkImage(p->image, "initial"));

    if (delayBeforeEnd) {
        QTest::qWait(delayBeforeEnd);
        QVERIFY(chk.checkImage(p->image, "moved"));
    }

    juggler.end();
    p->image->waitForDone();
    QVERIFY(chk.checkImage(p->image, "moved"));

    p->undoStore->undo();
    p->image->waitForDone();

    QVERIFY(chk.checkImage(p->image, "initial"));
}

void KisNodeJugglerCompressedTest::testApplyUndo()
{
    testMove(1000);
}

void KisNodeJugglerCompressedTest::testEndBeforeUpdate()
{
    testMove(0);
}

void KisNodeJugglerCompressedTest::testDuplicateImpl(bool externalParent, bool useMove)
{
    TestUtil::ReferenceImageChecker chk("node_juggler", "move_test");
    chk.setMaxFailingPixels(0);

    QStringList initialRef;
    initialRef << "paint1";
    initialRef << "paint2";
    initialRef << "paint3";
    initialRef << "group4";
    initialRef << "+paint5";
    initialRef << "paint6";

    QVERIFY(TestUtil::checkHierarchy(p->image->root(), initialRef));

    KisNodeList selectedNodes;
    selectedNodes << layer2;
    selectedNodes << layer3;
    selectedNodes << layer5;

    KisNodeJugglerCompressed juggler(kundo2_i18n("Duplicate Layers"), p->image, 0, 600);

    if (!externalParent) {
        juggler.duplicateNode(selectedNodes);
    } else {
        if (useMove) {
            juggler.moveNode(selectedNodes, p->image->root(), layer6);
        } else {
            juggler.copyNode(selectedNodes, p->image->root(), layer6);
        }
    }

    QTest::qWait(1000);

    juggler.end();
    p->image->waitForDone();

    QStringList ref;

    if (!externalParent) {
        ref << "paint1";
        ref << "paint2";
        ref << "paint3";
        ref << "group4";
        ref << "+paint5";
        ref << "+Copy of paint2";
        ref << "+Copy of paint3";
        ref << "+Copy of paint5";
        ref << "paint6";
    } else if (!useMove) {
        ref << "paint1";
        ref << "paint2";
        ref << "paint3";
        ref << "group4";
        ref << "+paint5";
        ref << "paint6";
        ref << "Copy of paint2";
        ref << "Copy of paint3";
        ref << "Copy of paint5";
    } else {
        ref << "paint1";
        ref << "group4";
        ref << "paint6";
        ref << "paint2";
        ref << "paint3";
        ref << "paint5";
    }

    QVERIFY(TestUtil::checkHierarchy(p->image->root(), ref));

    p->undoStore->undo();
    p->image->waitForDone();

    QVERIFY(TestUtil::checkHierarchy(p->image->root(), initialRef));
}

void KisNodeJugglerCompressedTest::testDuplicate()
{
    testDuplicateImpl(false, false);
}

void KisNodeJugglerCompressedTest::testCopyLayers()
{
    testDuplicateImpl(true, false);
}

void KisNodeJugglerCompressedTest::testMoveLayers()
{
    testDuplicateImpl(true, true);
}

SIMPLE_TEST_MAIN(KisNodeJugglerCompressedTest)
