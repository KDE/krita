/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NODE_TESTER_H
#define KIS_NODE_TESTER_H

#include <QtTest>

#include "kis_node.h"
#include <sdk/tests/testing_nodes.h>

class TestNode : public TestUtil::DefaultNode
{
    Q_OBJECT

public:
    KisNodeSP clone() {
        return new TestNode(*this);
    }

    using KisNode::setDirty;

    void setDirty() override {
        KisNode::setDirty(QRect(-1000, -1000, 2000, 200));
    }
    KisNodeSP clone() const override {
        return new TestNode(*this);
    }
};


class TestNodeA : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new TestNodeA(*this);
    }
};

class TestNodeB : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new TestNodeB(*this);
    }
};

class TestNodeC : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new TestNodeC(*this);
    }
};



class KisNodeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testOrdering();
    void testSetDirty();
    void testChildNodes();

    void propertiesStressTest();
    void graphStressTest();

private:
    class VisibilityKiller;
    class GraphKiller;

    template <class KillerClass>
        void propertiesStressTestImpl();
};

#endif

