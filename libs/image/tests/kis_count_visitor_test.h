/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COUNT_VISITOR_TESTER_H
#define KIS_COUNT_VISITOR_TESTER_H

#include <simpletest.h>

#include "kis_node.h"
#include <sdk/tests/testing_nodes.h>

class RootNode : public TestUtil::DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return new RootNode(*this);
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


class KisCountVisitorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCounting();
};

#endif

