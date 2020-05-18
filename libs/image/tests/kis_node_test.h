/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

