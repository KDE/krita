/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_projection_leaf_test.h"

#include <simpletest.h>

#define USE_DOCUMENT 0
#include "qimage_based_test.h"

#include "kis_projection_leaf.h"
#include "kis_group_layer.h"

#include "kistest.h"


struct TestImage : TestUtil::QImageBasedTest {
    TestImage() : TestUtil::QImageBasedTest("")
    {
        undoStore = new KisSurrogateUndoStore();
        image = createImage(undoStore);
        addGlobalSelection(image);
    }

    KisSurrogateUndoStore *undoStore;
    KisImageSP image;

    KisNodeSP findBlur1() {
        return findNode(image->root(), "blur1");
    }

    KisNodeSP findClone1() {
        return findNode(image->root(), "clone1");
    }

    KisNodeSP findPaint1() {
        return findNode(image->root(), "paint1");
    }
};

bool safeCompare(KisProjectionLeafSP leaf, KisNodeSP node)
{
    if (node && node->inherits("KisSelectionMask")) {
        return !leaf;
    }

    return (!leaf && !node) || (leaf->node() == node);
}

void checkNode(KisNodeSP node, const QString &prefix)
{
    qDebug() << prefix << node->name();

    if (!node->inherits("KisSelectionMask")) {
        safeCompare(node->projectionLeaf()->parent(), node->parent());
        safeCompare(node->projectionLeaf()->prevSibling(), node->prevSibling());
        safeCompare(node->projectionLeaf()->nextSibling(), node->nextSibling());
    }

    safeCompare(node->projectionLeaf()->firstChild(), node->firstChild());
    safeCompare(node->projectionLeaf()->lastChild(), node->lastChild());

    QCOMPARE(node->projectionLeaf()->node(), node);


    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        checkNode(prevNode, QString("\"\"%1").arg(prefix));
        prevNode = prevNode->prevSibling();
    }
}

void printNodes(KisNodeSP node, const QString &prefix = "")
{
    qDebug() << prefix << node->name();

    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        printNodes(prevNode, QString("\"\"%1").arg(prefix));
        prevNode = prevNode->prevSibling();
    }
}

void printLeafsBackward(KisProjectionLeafSP leaf, QList<QString> &refNodes, const QString &prefix = "")
{
    qDebug() << prefix << leaf->node()->name();
    QCOMPARE(leaf->node()->name(), refNodes.takeFirst());

    KisProjectionLeafSP prevLeaf = leaf->lastChild();
    while(prevLeaf) {
        printLeafsBackward(prevLeaf, refNodes, QString("\"\"%1").arg(prefix));
        prevLeaf = prevLeaf->prevSibling();
    }

    if (prefix == "") {
        QVERIFY(refNodes.isEmpty());
    }
}

void printLeafsForward(KisProjectionLeafSP leaf, QList<QString> &refNodes, const QString &prefix = "")
{
    qDebug() << prefix << leaf->node()->name();
    QCOMPARE(leaf->node()->name(), refNodes.takeFirst());

    KisProjectionLeafSP prevLeaf = leaf->firstChild();
    while(prevLeaf) {
        printLeafsForward(prevLeaf, refNodes, QString("\"\"%1").arg(prefix));
        prevLeaf = prevLeaf->nextSibling();
    }
}

void printParents(KisProjectionLeafSP leaf, QList<QString> &refNodes, const QString &prefix = "")
{
    qDebug() << prefix << leaf->node()->name();
    QCOMPARE(leaf->node()->name(), refNodes.takeFirst());

    leaf = leaf->parent();
    if (leaf) {
        printParents(leaf, refNodes, QString("\"\"%1").arg(prefix));
    }

    QVERIFY(refNodes.isEmpty());
}

void KisProjectionLeafTest::test()
{
    TestImage t;

    checkNode(t.image->root(), "");
}

void KisProjectionLeafTest::testPassThrough()
{
    TestImage t;

    KisGroupLayerSP group1 = new KisGroupLayer(t.image, "group1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint2 = new KisPaintLayer(t.image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint3 = new KisPaintLayer(t.image, "paint3", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint4 = new KisPaintLayer(t.image, "paint4", OPACITY_OPAQUE_U8);

    group1->setPassThroughMode(true);

    t.image->addNode(group1, t.image->root(), t.findBlur1());
    t.image->addNode(paint2, group1);
    t.image->addNode(paint3, group1);
    t.image->addNode(paint4, group1);

    //checkNode(t.image->root(), "");

    qDebug() << "== Nodes";
    printNodes(t.image->root());

    {
        qDebug() << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "paint1"
                 << "tmask1"
                 << "group1" << "paint4" << "paint3" << "paint2"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"
                 << "paint2" << "paint3" << "paint4" << "group1"
                 << "paint1"
                 << "tmask1";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for paint4";
        QList<QString> refNodes;
        refNodes << "paint4" << "root";
        printParents(paint4->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for paint3";
        QList<QString> refNodes;
        refNodes << "paint3" << "root";
        printParents(paint3->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for group1";
        QList<QString> refNodes;
        refNodes << "group1" << "root";
        printParents(group1->projectionLeaf(), refNodes);
    }
}

void KisProjectionLeafTest::testNestedPassThrough()
{
    TestImage t;

    KisGroupLayerSP group1 = new KisGroupLayer(t.image, "group1", OPACITY_OPAQUE_U8);
    KisGroupLayerSP group2 = new KisGroupLayer(t.image, "group2", OPACITY_OPAQUE_U8);
    KisGroupLayerSP group3 = new KisGroupLayer(t.image, "group3", OPACITY_OPAQUE_U8);

    KisPaintLayerSP paint4 = new KisPaintLayer(t.image, "paint4", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint5 = new KisPaintLayer(t.image, "paint5", OPACITY_OPAQUE_U8);

    group1->setPassThroughMode(true);
    group2->setPassThroughMode(true);
    group3->setPassThroughMode(true);

    t.image->addNode(group1, t.image->root(), t.findBlur1());
    t.image->addNode(group2, group1);
    t.image->addNode(paint4, group2);

    t.image->addNode(group3, t.image->root(), t.findBlur1());
    t.image->addNode(paint5, group3);

    //checkNode(t.image->root(), "");

    qDebug() << "== Nodes";
    printNodes(t.image->root());

    {
        qDebug() << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "paint1"
                 << "tmask1"
                 << "group1" << "group2" <<"paint4"
                 << "group3" << "paint5"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"

                 << "paint5" << "group3"
                 << "paint4" << "group2" << "group1"

                 << "paint1"
                 << "tmask1";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for paint4";
        QList<QString> refNodes;
        refNodes << "paint4" << "root";
        printParents(paint4->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for paint5";
        QList<QString> refNodes;
        refNodes << "paint5" << "root";
        printParents(paint5->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for group1";
        QList<QString> refNodes;
        refNodes << "group1" << "root";
        printParents(group1->projectionLeaf(), refNodes);
    }
}

#include "kis_selection_mask.h"
#include "kis_transparency_mask.h"

void KisProjectionLeafTest::testSkippedSelectionMasks()
{
    TestImage t;

    KisGroupLayerSP group1 = new KisGroupLayer(t.image, "group1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint2 = new KisPaintLayer(t.image, "paint2", OPACITY_OPAQUE_U8);
    KisSelectionMaskSP selection3 = new KisSelectionMask(t.image, "selection3");
    KisPaintLayerSP paint4 = new KisPaintLayer(t.image, "paint4", OPACITY_OPAQUE_U8);
    KisTransparencyMaskSP tmask5 = new KisTransparencyMask(t.image, "tmask5");
    KisSelectionMaskSP selection6 = new KisSelectionMask(t.image, "selection6");
    KisTransparencyMaskSP tmask7 = new KisTransparencyMask(t.image, "tmask7");
    KisPaintLayerSP paint8 = new KisPaintLayer(t.image, "paint8", OPACITY_OPAQUE_U8);
    KisSelectionMaskSP selection9 = new KisSelectionMask(t.image, "selection9");
    KisTransparencyMaskSP tmask10 = new KisTransparencyMask(t.image, "tmask10");

    t.image->addNode(group1, t.image->root(), t.findBlur1());

    t.image->addNode(paint2, group1);
    t.image->addNode(selection3, paint2);

    t.image->addNode(paint4, group1);
    t.image->addNode(tmask5, paint4);
    t.image->addNode(selection6, paint4);
    t.image->addNode(tmask7, paint4);

    t.image->addNode(paint8, group1);
    t.image->addNode(selection9, paint8);
    t.image->addNode(tmask10, paint8);

    //checkNode(t.image->root(), "");

    qDebug() << "== Nodes";
    printNodes(t.image->root());

    {
        qDebug() << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "paint1"
                 << "tmask1"
                 << "group1"
                     << "paint8" << "tmask10"
                     << "paint4" << "tmask7" << "tmask5"
                     << "paint2"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"
                 << "group1"
                     << "paint2"
                     << "paint4" << "tmask5" << "tmask7"
                     << "paint8" << "tmask10"
                 << "paint1"
                 << "tmask1";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for tmask5";
        QList<QString> refNodes;
        refNodes << "tmask5" << "paint4" << "group1" << "root";
        printParents(tmask5->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for selection6";
        QList<QString> refNodes;
        refNodes << "selection6";
        printParents(selection6->projectionLeaf(), refNodes);
    }


    /**
     * Selection masks are just excluded from the entire rendering hierarchy
     */
    QVERIFY(!selection6->projectionLeaf()->nextSibling());
    QVERIFY(!selection6->projectionLeaf()->prevSibling());
}

void KisProjectionLeafTest::testSelectionMaskOverlay()
{
    TestImage t;

    KisGroupLayerSP group1 = new KisGroupLayer(t.image, "group1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paint2 = new KisPaintLayer(t.image, "paint2", OPACITY_OPAQUE_U8);
    KisSelectionMaskSP selection3 = new KisSelectionMask(t.image);
    selection3->setName("selection3");
    selection3->setSelection(new KisSelection(new KisSelectionDefaultBounds(paint2->paintDevice())));

    t.image->addNode(group1, t.image->root(), t.findBlur1());

    t.image->addNode(paint2, group1);
    t.image->addNode(selection3, paint2);

    t.image->setOverlaySelectionMask(selection3);

    t.image->waitForDone();


    qDebug() << "== Nodes";
    printNodes(t.image->root());

    {
        qDebug() << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "selection3"
                 << "paint1"
                 << "tmask1"
                 << "group1"
                     << "paint2"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"
                 << "group1"
                     << "paint2"
                 << "paint1"
                 << "tmask1"
                 << "selection3";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        qDebug() << "== Parents for selection3";
        QList<QString> refNodes;
        refNodes << "selection3" << "root";
        printParents(selection3->projectionLeaf(), refNodes);
    }
}

KISTEST_MAIN(KisProjectionLeafTest)
