/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PSD_LAYER_STYLE_TEST_H
#define __KIS_PSD_LAYER_STYLE_TEST_H

#include <QtTest/QtTest>

class KisPSDLayerStyleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRoundTrip();
};

#endif
