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

#include <QtTest/QtTest>

#include "kis_node.h"

class TestNode : public KisNode
{
    Q_OBJECT

public:
    KisNodeSP clone() {
        return new TestNode(*this);
    }
    bool allowAsChild(KisNodeSP) const {
        return true;
    }
    const KoColorSpace * colorSpace() const {
        return 0;
    }
    virtual const KoCompositeOp * compositeOp() const {
        return 0;
    }

    using KisNode::setDirty;

    void setDirty() {
        KisNode::setDirty(QRect(-1000, -1000, 2000, 200));
    }
    KisNodeSP clone() const {
        return new TestNode(*this);
    }
};


class TestNodeA : public KisNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const {
        return new TestNodeA(*this);
    }
    bool allowAsChild(KisNodeSP) const {
        return true;
    }
    const KoColorSpace * colorSpace() const {
        return 0;
    }
    virtual const KoCompositeOp * compositeOp() const {
        return 0;
    }
};

class TestNodeB : public KisNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const {
        return new TestNodeB(*this);
    }
    bool allowAsChild(KisNodeSP) const {
        return true;
    }
    const KoColorSpace * colorSpace() const {
        return 0;
    }
    virtual const KoCompositeOp * compositeOp() const {
        return 0;
    }
};

class TestNodeC : public KisNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const {
        return new TestNodeC(*this);
    }
    bool allowAsChild(KisNodeSP) const {
        return true;
    }
    const KoColorSpace * colorSpace() const {
        return 0;
    }
    virtual const KoCompositeOp * compositeOp() const {
        return 0;
    }
};



class KisNodeTest : public QObject
{
    Q_OBJECT

private slots:

    void testCreation();
    void testOrdering();
    void testSetDirty();
    void testChildNodes();
    void testDirtyRegion();
};

#endif

