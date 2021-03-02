/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_multinode_property_test.h"

#include <simpletest.h>
#include <testutil.h>

#include <KoCompositeOpRegistry.h>

#include "kis_multinode_property.h"


void KisMultinodePropertyTest::test()
{
    TestUtil::MaskParent p;

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8);

    KisNodeList nodes;
    nodes << layer1;
    nodes << layer2;
    nodes << layer3;

    // Test uniform initial state
    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(prop.isIgnored(), false);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);

        prop.setValue(COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);

        prop.setIgnored(true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), false);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);
    }


    // Test non-uniform initial state
    layer1->setCompositeOpId(COMPOSITE_ALPHA_DARKEN);
    layer2->setCompositeOpId(COMPOSITE_OVER);
    layer3->setCompositeOpId(COMPOSITE_OVER);

    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(prop.isIgnored(), true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), false);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), true);

        prop.setValue(COMPOSITE_OVER);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), true);

        prop.setIgnored(true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), false);
    }

    // Test undo-redo
    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);

        QScopedPointer<KUndo2Command> cmd(prop.createPostExecutionUndoCommand());

        cmd->undo();

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);

        cmd->redo();

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
    }
}

SIMPLE_TEST_MAIN(KisMultinodePropertyTest)
