/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ADJUSTMENT_LAYER_TEST_H
#define KIS_ADJUSTMENT_LAYER_TEST_H

#include <QtTest>

class KisAdjustmentLayerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testCreation();
    void testSetSelection();
    void testInverted();
    void testSelectionParent();
};

#endif
