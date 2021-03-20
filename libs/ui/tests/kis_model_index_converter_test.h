/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MODEL_INDEX_CONVERTER_TEST_H
#define __KIS_MODEL_INDEX_CONVERTER_TEST_H

#include <simpletest.h>

#include "empty_nodes_test.h"

class KisDummiesFacadeBase;
class KisNodeModel;
class KisModelIndexConverterBase;


class KisModelIndexConverterTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testIndexFromDummy();
    void testIndexFromAddedAllowedDummy();
    void testIndexFromAddedDeniedDummy();
    void testDummyFromRow();
    void testRowCount();

    void testIndexFromDummyShowGlobalSelection();
    void testIndexFromAddedAllowedDummyShowGlobalSelection();
    void testIndexFromAddedDeniedDummyShowGlobalSelection();
    void testDummyFromRowShowGlobalSelection();
    void testRowCountShowGlobalSelection();

    void testIndexFromDummyShowAll();
    void testIndexFromAddedAllowedDummyShowAll();
    void testIndexFromAddedDeniedDummyShowAll();
    void testDummyFromRowShowAll();
    void testRowCountShowAll();

private:
    inline void checkIndexFromDummy(KisNodeSP node, int row);
    inline void checkInvalidIndexFromDummy(KisNodeSP node);
    inline void checkIndexFromAddedAllowedDummy(KisNodeSP parent, int index, int parentRow, int childRow, bool parentValid);
    inline void checkIndexFromAddedDeniedDummy(KisNodeSP parent, int index, int parentRow, int childRow, bool parentValid);
    inline void checkIndexFromAddedDummy(KisNodeSP parent, int index, const QString &type, int parentRow, int childRow, bool parentValid);
    inline void checkInvalidIndexFromAddedAllowedDummy(KisNodeSP parent, int index);
    inline void checkInvalidIndexFromAddedDeniedDummy(KisNodeSP parent, int index);
    inline void checkInvalidIndexFromAddedDummy(KisNodeSP parent, int index, const QString &type);
    inline void checkDummyFromRow(KisNodeSP parent, int row, KisNodeSP expectedNode);
    inline void checkRowCount(KisNodeSP parent, int rowCount);

private:
    KisDummiesFacadeBase *m_dummiesFacade;
    KisNodeModel *m_nodeModel;
    KisModelIndexConverterBase *m_indexConverter;
};

#endif /* __KIS_MODEL_INDEX_CONVERTER_TEST_H */
