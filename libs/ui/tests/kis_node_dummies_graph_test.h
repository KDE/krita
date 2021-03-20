/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_DUMMIES_GRAPH_TEST_H
#define __KIS_NODE_DUMMIES_GRAPH_TEST_H

#include <simpletest.h>

class KisNodeDummy;
class KisNodeDummiesGraph;


class KisNodeDummiesGraphTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testIndexing();

    void testPrepend();
    void testAppend();
    void testInsert();
    void testNewSubgraph();

    void testRemoveFirst();
    void testRemoveLast();
    void testRemoveBranch();

    void testReverseTraversing();

private:
    KisNodeDummy *m_rootDummy;
    KisNodeDummiesGraph *m_dummiesGraph;
};

#endif /* __KIS_NODE_DUMMIES_GRAPH_TEST_H */
