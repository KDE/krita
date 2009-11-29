
/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#include "kis_node_model_test.h"
#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kis_debug.h>

#include "kis_image.h"
#include "kis_types.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_node_model.h"
#include "KoColorSpace.h"
#include "KoCompositeOp.h"
#include "KoColorSpaceRegistry.h"
#include "modeltest.h"
/*
Image:

    root
        first
        second
        group 1
            child

node model

        group1     0
            child
        second     1
        first      2
    (root) // root is invisible

*/
void kisnodemodel_test::testRowcount()
{
    KisImageWSP image = new KisImage(0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage");

    KisNodeModel model(0);
    ModelTest(&model, this);
    model.setImage(image);

    KisLayerSP l1 = new KisPaintLayer(image, "first", 200, image->colorSpace());
    image->addNode(l1.data(), image->root());
    QCOMPARE(model.rowCount(), 1);

    KisLayerSP l2 = new KisPaintLayer(image, "second", 200, image->colorSpace());
    image->addNode(l2.data(), image->root());
    QVERIFY(model.rowCount() == 2);

    KisGroupLayerSP parent = new KisGroupLayer(image, "group 1", 200);
    image->addNode(parent, image->rootLayer());

    QVERIFY(model.rowCount() == 3);

    KisPaintLayerSP child = new KisPaintLayer(image, "child", 200);
    image->addNode(child, parent);

    QVERIFY(model.rowCount() == 3);

    QModelIndex idx = model.index(0, 0);
    dbgKrita << "node: " << model.nodeFromIndex(idx);
    QVERIFY(idx.isValid());
    QVERIFY(!idx.parent().isValid());
    QVERIFY(model.hasChildren(idx));
    QVERIFY(model.rowCount(idx) == 1);
}


/*
image:

    root
        first
        second

mode:

        second 0
        first  1

*/
void kisnodemodel_test::testModelIndex()
{
    KisImageWSP image = new KisImage(0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage");

    KisNodeModel model(0);
    ModelTest(&model, this);
    model.setImage(image);

    KisLayerSP first = new KisPaintLayer(image, "first", 200, image->colorSpace());
    image->addNode(first.data());
    QModelIndex idx = model.index(0, 0);

    QVERIFY(idx.isValid());
    QVERIFY(!idx.parent().isValid());
    QVERIFY(idx.internalPointer() == first.data());
    QVERIFY(model.hasChildren(idx) == false);

    KisLayerSP second = new KisPaintLayer(image, "second", 200, image->colorSpace());
    image->addNode(second.data());
    QModelIndex idx2 = model.index(0, 0);
    QVERIFY(idx2 != idx);
    idx = model.index(1, 0);

    QVERIFY(idx2.isValid());
    QVERIFY(!idx2.parent().isValid());
    QVERIFY(idx2.internalPointer() == second.data());
    QVERIFY(idx2.sibling(1, 0) == idx);
    QVERIFY(idx.sibling(0, 0) == idx2);
    QVERIFY(idx.model() == &model);


    /*
    image:

        root
            first
            second
            group
                child

    mode:
            group
                child
            second 0
            first  1
     */
    KisGroupLayerSP parent = new KisGroupLayer(image, "group 1", 200);
    image->addNode(parent, image->rootLayer());

    KisPaintLayerSP child = new KisPaintLayer(image, "child", 200);
    image->addNode(child, parent);

    QModelIndex parentIdx = model.index(0, 0);

    QVERIFY(parentIdx.isValid());
    QVERIFY(!parentIdx.parent().isValid());
    dbgKrita << model.nodeFromIndex(parentIdx);
    QVERIFY(parentIdx.internalPointer() == parent.data());
    QVERIFY(model.hasChildren(parentIdx));

    QModelIndex childIdx = parentIdx.child(0, 0);
    QVERIFY(childIdx.isValid());
    QVERIFY(childIdx.parent().isValid());
    QVERIFY(childIdx.parent() == parentIdx);
    QVERIFY(childIdx.internalPointer() == child.data());
    QVERIFY(childIdx.parent().internalPointer() == parent.data());

    QModelIndex childIdx2 = model.index(0, 0, parentIdx);
    QVERIFY(childIdx2.isValid());
    QVERIFY(childIdx2.parent().isValid());
    QVERIFY(childIdx2.parent() == parentIdx);
    QVERIFY(childIdx2.internalPointer() == child.data());
    QVERIFY(childIdx2.parent().internalPointer() == parent.data());

    idx = model.index(-1, 0);
    QVERIFY(!idx.isValid());
}

void kisnodemodel_test::testGroupLayers()
{
    KisImageWSP image = new KisImage(0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage");

    KisNodeModel model(0);
    ModelTest(&model, this);

    model.setImage(image);

    KisLayerSP first = new KisPaintLayer(image, "first", 200, image->colorSpace());
    KisLayerSP second = new KisPaintLayer(image, "second", 200, image->colorSpace());

    KisGroupLayerSP parent = new KisGroupLayer(image, "group 1", 200);
    image->addNode(parent, image->rootLayer());

    KisPaintLayerSP child = new KisPaintLayer(image, "child", 200);
    image->addNode(child, parent);

    QModelIndex parentIdx = model.index(0, 0);
    QVERIFY(model.hasChildren(parentIdx));

    QModelIndex childIdx2 = model.index(0, 0, parentIdx);
    QVERIFY(childIdx2.isValid());
    QVERIFY(childIdx2.parent().isValid());
    QVERIFY(childIdx2.parent() == parentIdx);
    QVERIFY(childIdx2.internalPointer() == child.data());
    QVERIFY(childIdx2.parent().internalPointer() == parent.data());

}

QTEST_KDEMAIN(kisnodemodel_test, GUI)

#include "kis_node_model_test.moc"

