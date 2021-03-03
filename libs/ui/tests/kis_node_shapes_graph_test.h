/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_SHAPES_GRAPH_TEST_H
#define __KIS_NODE_SHAPES_GRAPH_TEST_H

#include <simpletest.h>

class KisNodeDummy;
class KisNodeShapesGraph;

class KisNodeShapesGraphTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testShapeChildren();
    void testInsert();
    void testRemove();
    void testRemoveRootNode();

private:
    KisNodeDummy *m_rootDummy;
    KisNodeShapesGraph *m_shapesGraph;
};

#endif /* __KIS_NODE_SHAPES_GRAPH_TEST_H */
