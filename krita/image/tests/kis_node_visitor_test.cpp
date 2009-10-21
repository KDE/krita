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

#include <qtest_kde.h>
#include "kis_node_visitor.h"

class TestNodeVisitor : public KisNodeVisitor
{
public:

    TestNodeVisitor() {}

    ~TestNodeVisitor() {}

    bool visit(KisNode *node) {
        return true;
    }

    bool visit(KisPaintLayer *layer) {
        return true;
    }

    bool visit(KisGroupLayer *layer) {
        return true;
    }

    bool visit(KisAdjustmentLayer *layer) {
        return true;
    }

    bool visit(KisExternalLayer *layer) {
        return true;
    }

    bool visit(KisGeneratorLayer *layer) {
        return true;
    }

    bool visit(KisCloneLayer *layer) {
        return true;
    }

    bool visit(KisFilterMask *mask) {
        return true;
    }

    bool visit(KisTransparencyMask *mask) {
        return true;
    }

    bool visit(KisTransformationMask *mask) {
        return true;
    }

    bool visit(KisSelectionMask *mask) {
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


QTEST_KDEMAIN(KisNodeVisitorTest, GUI)
#include "kis_node_visitor_test.moc"
