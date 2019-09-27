/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_NODE_JUGGLER_COMPRESSED_TEST_H
#define __KIS_NODE_JUGGLER_COMPRESSED_TEST_H

#include <QtTest>
#include "testutil.h"
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
