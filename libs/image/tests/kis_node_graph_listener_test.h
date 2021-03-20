/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NODE_GRAPH_LISTENER_TEST_H
#define KIS_NODE_GRAPH_LISTENER_TEST_H

#include <simpletest.h>

#include "kis_node.h"
#include <sdk/tests/testing_nodes.h>

class TestNode : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new TestNode(*this);
    }
};


class KisNodeGraphListenerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testUpdateOfListener();
    void testRecursiveUpdateOfListener();
    void testSequenceNumber();
};

#endif
