/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_count_visitor_test.h"
#include <simpletest.h>
#include <QStringList>
#include <KoProperties.h>

#include "kis_node_facade.h"

#include "kis_types.h"
#include "kis_count_visitor.h"
#include "kis_image.h"
/*

  root
     TestNodeA
        TestNodeA
           TestNodeA
     TestNodeB
        TestNodeB
          TestNodeB
     TestNodeC
        TestNodeC
           TestNodeC

 */
void KisCountVisitorTest::testCounting()
{

    KisNodeSP root = new RootNode();
    KisNodeFacade facade(root);

    facade.addNode(new TestNodeA());
    facade.addNode(new TestNodeB());
    facade.addNode(new TestNodeC());

    QVERIFY(root->at(0)->inherits("TestNodeA"));
    root->at(0)->setName("TestNodeA 0");
    QVERIFY(root->at(1)->inherits("TestNodeB"));
    root->at(1)->setName("TestNodeB 1");
    QVERIFY(root->at(2)->inherits("TestNodeC"));
    root->at(2)->setName("TestNodeC 2");

    facade.addNode(new TestNodeA(), root->at(0));
    root->at(0)->at(0)->setName("TestNodeA 0.0");
    facade.addNode(new TestNodeB(), root->at(1));
    root->at(1)->at(0)->setName("TestNodeB 1.0");
    facade.addNode(new TestNodeC(), root->at(2));
    root->at(2)->at(0)->setName("TestNodeC 2.0");

    facade.addNode(new TestNodeA(), root->at(0)->at(0));
    root->at(0)->at(0)->at(0)->setName("TestNodeA 0.0.0");
    facade.addNode(new TestNodeB(), root->at(1)->at(0));
    root->at(1)->at(0)->at(0)->setName("TestNodeB 1.0.0");
    facade.addNode(new TestNodeC(), root->at(2)->at(0));
    root->at(2)->at(0)->at(0)->setName("TestNodeC 2.0.0");

    facade.addNode(new TestNodeA(), root->at(0)->at(0)->at(0));
    root->at(0)->at(0)->at(0)->at(0)->setName("TestNodeA 0.0.0.0");
    facade.addNode(new TestNodeB(), root->at(1)->at(0)->at(0));
    root->at(1)->at(0)->at(0)->at(0)->setName("TestNodeB 1.0.0.0");
    facade.addNode(new TestNodeC(), root->at(2)->at(0)->at(0));
    root->at(2)->at(0)->at(0)->at(0)->setName("TestNodeC 2.0.0.0");

    // Count all nodes
    {
        KisCountVisitor v = KisCountVisitor(QStringList(), KoProperties());
        root->accept(v);
        QVERIFY(v.count() == 13);
    }

    // Count all nodes of type A without taking properties in mind
    {
        QStringList types;
        types << "TestNodeA";
        KisCountVisitor v(types, KoProperties());
        root->accept(v);
        QVERIFY(v.count() == 4);
    }

    // Count all nodes of type A and B
    {
        QStringList types;
        types << "TestNodeA" << "TestNodeB";
        KisCountVisitor v(types, KoProperties());
        root->accept(v);
        QVERIFY(v.count() == 8);
    }

    // Count all nodes of type A and B and C
    {
        QStringList types;
        types << "TestNodeA" << "TestNodeB" << "TestNodeC";
        KisCountVisitor v(types, KoProperties());
        root->accept(v);
        QVERIFY(v.count() == 12);
    }

    // Count all nodes of type A that are visible.
    root->at(0)->setVisible(true);
    root->at(0)->at(0)->setVisible(true);
    root->at(0)->at(0)->at(0)->setVisible(true);
    root->at(0)->at(0)->at(0)->at(0)->setVisible(true);

    // Count all nodes of type A that are visible
    {
        QStringList types;
        types << "TestNodeA";
        KoProperties props;
        props.setProperty("visible", true);
        KisCountVisitor v(types, props);
        root->accept(v);
        QCOMPARE((int)v.count(), 4);
    }

    // Count all nodes of type A that are visible
    {
        root->at(0)->at(0)->setVisible(false);
        QStringList types;
        types << "TestNodeA";
        KoProperties props;
        props.setProperty("visible", true);
        KisCountVisitor v(types, props);
        root->accept(v);
        QCOMPARE((int)v.count(), 3);
    }
}

SIMPLE_TEST_MAIN(KisCountVisitorTest)


