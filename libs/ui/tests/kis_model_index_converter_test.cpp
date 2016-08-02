/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_model_index_converter_test.h"

#include <QTest>

#include "kis_node_model.h"
#include "kis_dummies_facade.h"
#include "kis_node_dummies_graph.h"
#include "kis_model_index_converter.h"
#include "kis_model_index_converter_show_all.h"


void KisModelIndexConverterTest::init()
{
    m_dummiesFacade = new KisDummiesFacade(0);
    m_nodeModel = new KisNodeModel(0);
    initBase();
    constructImage();
    addSelectionMasks();

    m_dummiesFacade->setImage(m_image);
    m_nodeModel->setDummiesFacade(m_dummiesFacade, m_image, 0, 0, 0);
}

void KisModelIndexConverterTest::cleanup()
{
    m_nodeModel->setDummiesFacade(0, 0, 0, 0, 0);
    m_dummiesFacade->setImage(0);

    cleanupBase();
    delete m_indexConverter;
    delete m_nodeModel;
    delete m_dummiesFacade;
}

inline void KisModelIndexConverterTest::checkIndexFromDummy(KisNodeSP node, int row) {
    QModelIndex index;
    KisNodeDummy *dummy;

    dummy = m_dummiesFacade->dummyForNode(node);
    index = m_indexConverter->indexFromDummy(dummy);

    QVERIFY(index.isValid());
    QCOMPARE(index.column(), 0);
    QCOMPARE(index.row(), row);
    QCOMPARE(m_indexConverter->dummyFromIndex(index), dummy);
}

inline void KisModelIndexConverterTest::checkInvalidIndexFromDummy(KisNodeSP node) {
    QModelIndex index;
    KisNodeDummy *dummy;

    dummy = m_dummiesFacade->dummyForNode(node);
    index = m_indexConverter->indexFromDummy(dummy);

    QVERIFY(!index.isValid());
}

inline void KisModelIndexConverterTest::checkIndexFromAddedAllowedDummy(KisNodeSP parent, int index, int parentRow, int childRow, bool parentValid)
{
    QString type = KisLayer::staticMetaObject.className();
    checkIndexFromAddedDummy(parent, index, type, parentRow, childRow, parentValid);
}

inline void KisModelIndexConverterTest::checkIndexFromAddedDeniedDummy(KisNodeSP parent, int index, int parentRow, int childRow, bool parentValid)
{
    QString type = KisSelectionMask::staticMetaObject.className();
    checkIndexFromAddedDummy(parent, index, type, parentRow, childRow, parentValid);
}

inline void KisModelIndexConverterTest::checkIndexFromAddedDummy(KisNodeSP parent, int index, const QString &type, int parentRow, int childRow, bool parentValid)
{
   QModelIndex modelIndex;
   KisNodeDummy *dummy;

   int row = 0;
   bool result;

   dummy = parent ? m_dummiesFacade->dummyForNode(parent) : 0;
   result = m_indexConverter->indexFromAddedDummy(dummy, index, type, modelIndex, row);
   if(!result) dbgKrita << "Failing parent:" << (parent ? parent->name() : "none") << "index:" << index;
   QVERIFY(result);

   QCOMPARE(modelIndex.isValid(), parentValid);

   if(modelIndex.isValid()) {
       QCOMPARE(modelIndex.row(), parentRow);
       QCOMPARE(modelIndex.column(), 0);
   }

   if(row != childRow) dbgKrita << "Failing parent:" << (parent ? parent->name() : "none") << "index:" << index;
   QCOMPARE(row, childRow);
}

inline void KisModelIndexConverterTest::checkInvalidIndexFromAddedAllowedDummy(KisNodeSP parent, int index)
{
    QString type = KisLayer::staticMetaObject.className();
    checkInvalidIndexFromAddedDummy(parent, index, type);
}

inline void KisModelIndexConverterTest::checkInvalidIndexFromAddedDeniedDummy(KisNodeSP parent, int index)
{
    QString type = KisSelectionMask::staticMetaObject.className();
    checkInvalidIndexFromAddedDummy(parent, index, type);
}

inline void KisModelIndexConverterTest::checkInvalidIndexFromAddedDummy(KisNodeSP parent, int index, const QString &type)
{
    QModelIndex modelIndex;
    KisNodeDummy *dummy;

    int row = 0;
    bool result;

    dummy = parent ? m_dummiesFacade->dummyForNode(parent) : 0;
    result = m_indexConverter->indexFromAddedDummy(dummy, index, type, modelIndex, row);
    QVERIFY(!result);
}

inline void KisModelIndexConverterTest::checkDummyFromRow(KisNodeSP parent, int row, KisNodeSP expectedNode)
{
    QModelIndex parentIndex;
    KisNodeDummy *parentDummy;

    if(parent) {
        parentDummy = m_dummiesFacade->dummyForNode(parent);
        parentIndex = m_indexConverter->indexFromDummy(parentDummy);
    }

    KisNodeDummy *resultDummy = m_indexConverter->dummyFromRow(row, parentIndex);
    KisNodeSP resultNode = resultDummy ? resultDummy->node() : 0;

    if(resultNode != expectedNode) {
        dbgKrita << "Actual node:  " << (resultNode ? resultNode->name() : "none");
        dbgKrita << "Expected node:" << (expectedNode ? expectedNode->name() : "none");
        QFAIL("Wrong node");
    }
}

inline void KisModelIndexConverterTest::checkRowCount(KisNodeSP parent, int rowCount)
{
    QModelIndex parentIndex;
    KisNodeDummy *parentDummy;

    if(parent) {
        parentDummy = m_dummiesFacade->dummyForNode(parent);
        parentIndex = m_indexConverter->indexFromDummy(parentDummy);
    }

    int resultRowCount = m_indexConverter->rowCount(parentIndex);

    if(resultRowCount != rowCount) {
        dbgKrita << "Wrong row count for:" << (parent ? parent->name() : "none");
        dbgKrita << "Actual:  " << resultRowCount;
        dbgKrita << "Expected:" << rowCount;
        QFAIL("Wrong row count");
    }
}

void KisModelIndexConverterTest::testIndexFromDummy()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, false);

    checkIndexFromDummy(m_layer1, 3);
    checkIndexFromDummy(m_layer2, 2);
    checkIndexFromDummy(m_layer3, 1);
    checkIndexFromDummy(m_layer4, 0);

    checkIndexFromDummy(m_mask1, 1);
    checkIndexFromDummy(m_sel3, 0);

    checkInvalidIndexFromDummy(m_image->root());
    checkInvalidIndexFromDummy(m_sel1);
    checkInvalidIndexFromDummy(m_sel2);
}

void KisModelIndexConverterTest::testIndexFromAddedAllowedDummy()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, false);

    checkIndexFromAddedAllowedDummy(m_image->root(), 0, 0, 4, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 1, 0, 4, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 2, 0, 3, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 3, 0, 2, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 4, 0, 2, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 5, 0, 1, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 6, 0, 0, false);

    checkIndexFromAddedAllowedDummy(m_layer1, 0, 3, 0, true);

    checkIndexFromAddedAllowedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 2, 1, 0, true);

    checkInvalidIndexFromAddedAllowedDummy(0, 0);
}

void KisModelIndexConverterTest::testIndexFromAddedDeniedDummy()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, false);

    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 0);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 1);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 2);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 3);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 4);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 5);
    checkInvalidIndexFromAddedDeniedDummy(m_image->root(), 6);

    checkIndexFromAddedDeniedDummy(m_layer1, 0, 3, 0, true);

    checkIndexFromAddedDeniedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 2, 1, 0, true);

    checkInvalidIndexFromAddedDeniedDummy(0, 0);
}

void KisModelIndexConverterTest::testDummyFromRow()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, false);

    checkDummyFromRow(m_image->root(), 0, m_layer4);
    checkDummyFromRow(m_image->root(), 1, m_layer3);
    checkDummyFromRow(m_image->root(), 2, m_layer2);
    checkDummyFromRow(m_image->root(), 3, m_layer1);

    checkDummyFromRow(0, 0, m_layer4);
    checkDummyFromRow(0, 1, m_layer3);
    checkDummyFromRow(0, 2, m_layer2);
    checkDummyFromRow(0, 3, m_layer1);

    checkDummyFromRow(m_layer3, 0, m_sel3);
    checkDummyFromRow(m_layer3, 1, m_mask1);
}

void KisModelIndexConverterTest::testRowCount()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, false);

    checkRowCount(m_image->root(), 4);
    checkRowCount(0, 4);

    checkRowCount(m_layer1, 0);
    checkRowCount(m_layer2, 0);
    checkRowCount(m_layer3, 2);
    checkRowCount(m_layer4, 0);
}

void KisModelIndexConverterTest::testIndexFromDummyShowGlobalSelection()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, true);

    checkIndexFromDummy(m_sel1,   5);
    checkIndexFromDummy(m_layer1, 4);
    checkIndexFromDummy(m_layer2, 3);
    checkIndexFromDummy(m_sel2,   2);
    checkIndexFromDummy(m_layer3, 1);
    checkIndexFromDummy(m_layer4, 0);

    checkIndexFromDummy(m_mask1, 1);
    checkIndexFromDummy(m_sel3, 0);

    checkInvalidIndexFromDummy(m_image->root());
}

void KisModelIndexConverterTest::testIndexFromAddedAllowedDummyShowGlobalSelection()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, true);

    checkIndexFromAddedAllowedDummy(m_image->root(), 0, 0, 6, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 1, 0, 5, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 2, 0, 4, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 3, 0, 3, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 4, 0, 2, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 5, 0, 1, false);
    checkIndexFromAddedAllowedDummy(m_image->root(), 6, 0, 0, false);

    checkIndexFromAddedAllowedDummy(m_layer1, 0, 4, 0, true);

    checkIndexFromAddedAllowedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 2, 1, 0, true);

    checkInvalidIndexFromAddedAllowedDummy(0, 0);
}

void KisModelIndexConverterTest::testIndexFromAddedDeniedDummyShowGlobalSelection()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, true);

    checkIndexFromAddedDeniedDummy(m_image->root(), 0, 0, 6, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 1, 0, 5, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 2, 0, 4, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 3, 0, 3, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 4, 0, 2, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 5, 0, 1, false);
    checkIndexFromAddedDeniedDummy(m_image->root(), 6, 0, 0, false);

    checkIndexFromAddedDeniedDummy(m_layer1, 0, 4, 0, true);

    checkIndexFromAddedDeniedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 2, 1, 0, true);

    checkInvalidIndexFromAddedDeniedDummy(0, 0);
}

void KisModelIndexConverterTest::testDummyFromRowShowGlobalSelection()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, true);

    checkDummyFromRow(m_image->root(), 0, m_layer4);
    checkDummyFromRow(m_image->root(), 1, m_layer3);
    checkDummyFromRow(m_image->root(), 2, m_sel2);
    checkDummyFromRow(m_image->root(), 3, m_layer2);
    checkDummyFromRow(m_image->root(), 4, m_layer1);
    checkDummyFromRow(m_image->root(), 5, m_sel1);

    checkDummyFromRow(0, 0, m_layer4);
    checkDummyFromRow(0, 1, m_layer3);
    checkDummyFromRow(0, 2, m_sel2);
    checkDummyFromRow(0, 3, m_layer2);
    checkDummyFromRow(0, 4, m_layer1);
    checkDummyFromRow(0, 5, m_sel1);

    checkDummyFromRow(m_layer3, 0, m_sel3);
    checkDummyFromRow(m_layer3, 1, m_mask1);
}

void KisModelIndexConverterTest::testRowCountShowGlobalSelection()
{
    m_indexConverter = new KisModelIndexConverter(m_dummiesFacade, m_nodeModel, true);

    checkRowCount(m_image->root(), 6);
    checkRowCount(0, 6);

    checkRowCount(m_layer1, 0);
    checkRowCount(m_layer2, 0);
    checkRowCount(m_layer3, 2);
    checkRowCount(m_layer4, 0);
}

void KisModelIndexConverterTest::testIndexFromDummyShowAll()
{
    m_indexConverter = new KisModelIndexConverterShowAll(m_dummiesFacade, m_nodeModel);

    checkIndexFromDummy(m_sel1,   5);
    checkIndexFromDummy(m_layer1, 4);
    checkIndexFromDummy(m_layer2, 3);
    checkIndexFromDummy(m_sel2,   2);
    checkIndexFromDummy(m_layer3, 1);
    checkIndexFromDummy(m_layer4, 0);

    checkIndexFromDummy(m_mask1, 1);
    checkIndexFromDummy(m_sel3, 0);

    checkIndexFromDummy(m_image->root(), 0);
}

void KisModelIndexConverterTest::testIndexFromAddedAllowedDummyShowAll()
{
    m_indexConverter = new KisModelIndexConverterShowAll(m_dummiesFacade, m_nodeModel);

    checkIndexFromAddedAllowedDummy(m_image->root(), 0, 0, 6, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 1, 0, 5, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 2, 0, 4, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 3, 0, 3, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 4, 0, 2, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 5, 0, 1, true);
    checkIndexFromAddedAllowedDummy(m_image->root(), 6, 0, 0, true);

    checkIndexFromAddedAllowedDummy(m_layer1, 0, 4, 0, true);

    checkIndexFromAddedAllowedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedAllowedDummy(m_layer3, 2, 1, 0, true);

    checkIndexFromAddedAllowedDummy(0, 0, 0, 0, false);
}

void KisModelIndexConverterTest::testIndexFromAddedDeniedDummyShowAll()
{
    m_indexConverter = new KisModelIndexConverterShowAll(m_dummiesFacade, m_nodeModel);

    checkIndexFromAddedDeniedDummy(m_image->root(), 0, 0, 6, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 1, 0, 5, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 2, 0, 4, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 3, 0, 3, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 4, 0, 2, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 5, 0, 1, true);
    checkIndexFromAddedDeniedDummy(m_image->root(), 6, 0, 0, true);

    checkIndexFromAddedDeniedDummy(m_layer1, 0, 4, 0, true);

    checkIndexFromAddedDeniedDummy(m_layer3, 0, 1, 2, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 1, 1, 1, true);
    checkIndexFromAddedDeniedDummy(m_layer3, 2, 1, 0, true);

    checkIndexFromAddedDeniedDummy(0, 0, 0, 0, false);
}

void KisModelIndexConverterTest::testDummyFromRowShowAll()
{
    m_indexConverter = new KisModelIndexConverterShowAll(m_dummiesFacade, m_nodeModel);

    checkDummyFromRow(m_image->root(), 0, m_layer4);
    checkDummyFromRow(m_image->root(), 1, m_layer3);
    checkDummyFromRow(m_image->root(), 2, m_sel2);
    checkDummyFromRow(m_image->root(), 3, m_layer2);
    checkDummyFromRow(m_image->root(), 4, m_layer1);
    checkDummyFromRow(m_image->root(), 5, m_sel1);

    checkDummyFromRow(0, 0, m_image->root());

    checkDummyFromRow(m_layer3, 0, m_sel3);
    checkDummyFromRow(m_layer3, 1, m_mask1);
}


void KisModelIndexConverterTest::testRowCountShowAll()
{
    m_indexConverter = new KisModelIndexConverterShowAll(m_dummiesFacade, m_nodeModel);

    checkRowCount(m_image->root(), 6);
    checkRowCount(0, 1);

    checkRowCount(m_layer1, 0);
    checkRowCount(m_layer2, 0);
    checkRowCount(m_layer3, 2);
    checkRowCount(m_layer4, 0);
}

QTEST_MAIN(KisModelIndexConverterTest)
