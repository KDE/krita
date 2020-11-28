/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_JUGGLER_COMPRESSED_TEST_H
#define __KIS_NODE_JUGGLER_COMPRESSED_TEST_H

#include <QtTest>
#include <testutil.h>
#include "kis_group_layer.h"

class KisNodeJugglerCompressedTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testApplyUndo();
    void testEndBeforeUpdate();

    void testDuplicate();
    void testCopyLayers();
    void testMoveLayers();

private:
    void testMove(int delayBeforeEnd);
    void testDuplicateImpl(bool externalParent, bool useMove);
private:
    QScopedPointer<TestUtil::MaskParent> p;
    KisPaintLayerSP layer1;
    KisPaintLayerSP layer2;
    KisPaintLayerSP layer3;
    KisGroupLayerSP group4;
    KisPaintLayerSP layer5;
    KisPaintLayerSP layer6;
};

#endif /* __KIS_NODE_JUGGLER_COMPRESSED_TEST_H */
