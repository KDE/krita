/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_projection_leaf_test.h"

#include <qtest_kde.h>
#include "qimage_based_test.h"
#include "kis_projection_leaf.h"
#include "kis_group_layer.h"



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
    return (!leaf && !node) || (leaf->node() == node);
}

void checkNode(KisNodeSP node, const QString &prefix)
{
    dbgKrita << prefix << node->name();

    safeCompare(node->projectionLeaf()->parent(), node->parent());
    safeCompare(node->projectionLeaf()->firstChild(), node->firstChild());
    safeCompare(node->projectionLeaf()->lastChild(), node->lastChild());
    safeCompare(node->projectionLeaf()->prevSibling(), node->prevSibling());
    safeCompare(node->projectionLeaf()->nextSibling(), node->nextSibling());

    QCOMPARE(node->projectionLeaf()->node(), node);


    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        checkNode(prevNode, QString("\"\"%1").arg(prefix));
        prevNode = prevNode->prevSibling();
    }
}

void printNodes(KisNodeSP node, const QString &prefix = "")
{
    dbgKrita << prefix << node->name();

    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        printNodes(prevNode, QString("\"\"%1").arg(prefix));
        prevNode = prevNode->prevSibling();
    }
}

void printLeafsBackward(KisProjectionLeafSP leaf, QList<QString> &refNodes, const QString &prefix = "")
{
    dbgKrita << prefix << leaf->node()->name();
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
    dbgKrita << prefix << leaf->node()->name();
    QCOMPARE(leaf->node()->name(), refNodes.takeFirst());

    KisProjectionLeafSP prevLeaf = leaf->firstChild();
    while(prevLeaf) {
        printLeafsForward(prevLeaf, refNodes, QString("\"\"%1").arg(prefix));
        prevLeaf = prevLeaf->nextSibling();
    }
}

void printParents(KisProjectionLeafSP leaf, QList<QString> &refNodes, const QString &prefix = "")
{
    dbgKrita << prefix << leaf->node()->name();
    QCOMPARE(leaf->node()->name(), refNodes.takeFirst());

    leaf = leaf->parent();
    if (leaf) {
        printParents(leaf, refNodes, QString("\"\"%1").arg(prefix));
    }
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

    dbgKrita << "== Nodes";
    printNodes(t.image->root());

    {
        dbgKrita << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "selection"
                 << "paint1"
                 << "tmask1"
                 << "group1" << "paint4" << "paint3" << "paint2"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"
                 << "paint2" << "paint3" << "paint4" << "group1"
                 << "paint1"
                 << "tmask1"
                 << "selection";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for paint4";
        QList<QString> refNodes;
        refNodes << "paint4" << "root";
        printParents(paint4->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for paint3";
        QList<QString> refNodes;
        refNodes << "paint3" << "root";
        printParents(paint3->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for group1";
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

    dbgKrita << "== Nodes";
    printNodes(t.image->root());

    {
        dbgKrita << "== Leafs backward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "selection"
                 << "paint1"
                 << "tmask1"
                 << "group1" << "group2" <<"paint4"
                 << "group3" << "paint5"
                 << "blur1"
                 << "clone1";

        printLeafsBackward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Leafs forward";

        QList<QString> refNodes;
        refNodes << "root"
                 << "clone1"
                 << "blur1"

                 << "paint5" << "group3"
                 << "paint4" << "group2" << "group1"

                 << "paint1"
                 << "tmask1"
                 << "selection";

        printLeafsForward(t.image->root()->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for paint4";
        QList<QString> refNodes;
        refNodes << "paint4" << "root";
        printParents(paint4->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for paint5";
        QList<QString> refNodes;
        refNodes << "paint5" << "root";
        printParents(paint5->projectionLeaf(), refNodes);
    }

    {
        dbgKrita << "== Parents for group1";
        QList<QString> refNodes;
        refNodes << "group1" << "root";
        printParents(group1->projectionLeaf(), refNodes);
    }
}

QTEST_KDEMAIN(KisProjectionLeafTest, GUI)
