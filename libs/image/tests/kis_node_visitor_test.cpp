/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_visitor_test.h"

#include <QTest>
#include "kis_node_visitor.h"

class TestNodeVisitor : public KisNodeVisitor
{
public:

    TestNodeVisitor() {}

    ~TestNodeVisitor() override {}

    bool visit(KisNode */*node*/) override {
        return true;
    }

    bool visit(KisPaintLayer */*layer*/) override {
        return true;
    }

    bool visit(KisGroupLayer */*layer*/) override {
        return true;
    }

    bool visit(KisAdjustmentLayer */*layer*/) override {
        return true;
    }

    bool visit(KisExternalLayer */*layer*/) override {
        return true;
    }

    bool visit(KisGeneratorLayer */*layer*/) override {
        return true;
    }

    bool visit(KisCloneLayer */*layer*/) override {
        return true;
    }

    bool visit(KisFilterMask */*mask*/) override {
        return true;
    }

    bool visit(KisTransformMask */*mask*/) override {
        return true;
    }

    bool visit(KisTransparencyMask */*mask*/) override {
        return true;
    }

    bool visit(KisSelectionMask */*mask*/) override {
        return true;
    }

    bool visit(KisColorizeMask */*mask*/) override {
        return true;
    }

};

void KisNodeVisitorTest::testCreation()
{
    TestNodeVisitor v;
}

void KisNodeVisitorTest::testFullImage()
{
}


QTEST_MAIN(KisNodeVisitorTest)
