/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINT_LAYER_TEST_H
#define KIS_PAINT_LAYER_TEST_H

#include <simpletest.h>

class KisPaintLayerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testProjection();

    void testKeyframing();

    void testLayerStyles();

};

#endif

