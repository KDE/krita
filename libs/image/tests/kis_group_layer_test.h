/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GROUP_LAYER_TEST_H
#define KIS_GROUP_LAYER_TEST_H

#include <QtTest>

class KisGroupLayerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testProjection();

    void testRemoveAndUndo();

};

#endif

