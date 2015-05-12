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



struct TestImage : TestUtil::QImageBasedTest {
    TestImage() : TestUtil::QImageBasedTest("")
    {
        undoStore = new KisSurrogateUndoStore();
        image = createImage(undoStore);
        addGlobalSelection(image);
    }

    KisSurrogateUndoStore *undoStore;
    KisImageSP image;
};

bool safeCompare(KisProjectionLeafSP leaf, KisNodeSP node)
{
    return (!leaf && !node) || (leaf->node() == node);
}

void checkNode(KisNodeSP node, const QString &prefix)
{
    qDebug() << prefix << node->name();

    safeCompare(node->projectionLeaf()->parent(), node->parent());
    safeCompare(node->projectionLeaf()->firstChild(), node->firstChild());
    safeCompare(node->projectionLeaf()->lastChild(), node->lastChild());
    safeCompare(node->projectionLeaf()->prevSibling(), node->prevSibling());
    safeCompare(node->projectionLeaf()->nextSibling(), node->nextSibling());

    QCOMPARE(node->projectionLeaf()->childCount(), (int)node->childCount());
    QCOMPARE(node->projectionLeaf()->node(), node);


    KisNodeSP prevNode = node->lastChild();
    while(prevNode) {
        checkNode(prevNode, QString("\"\"%1").arg(prefix));
        prevNode = prevNode->prevSibling();
    }
}

void KisProjectionLeafTest::test()
{
    TestImage t;

    checkNode(t.image->root(), "");
}

QTEST_KDEMAIN(KisProjectionLeafTest, GUI)
