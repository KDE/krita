/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NODE_FACADE_TESTER_H
#define KIS_NODE_FACADE_TESTER_H

#include <QtTest>

#include "kis_node.h"
#include <sdk/tests/testing_nodes.h>

class TestNodeA : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new TestNodeA(*this);
    }
};


class KisNodeFacadeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testOrdering();
    void testMove();
};

#endif

