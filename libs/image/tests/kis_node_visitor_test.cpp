/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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
