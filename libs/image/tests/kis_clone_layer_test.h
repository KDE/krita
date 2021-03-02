/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CLONE_LAYER_TEST_H
#define KIS_CLONE_LAYER_TEST_H

#include <simpletest.h>

class KisCloneLayerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testOriginalUpdates();
    void testOriginalUpdatesOutOfBounds();
    void testOriginalRefresh();

    void testRemoveSourceLayer();
    void testRemoveSourceLayerParent();
    void testUndoingRemovingSource();

    void testDuplicateGroup();

    void testCyclingGroupLayer();
};

#endif
