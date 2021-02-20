/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLES_TEST_H
#define __KIS_LAYER_STYLES_TEST_H

#include <QtTest/QtTest>

class KisLayerStylesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLayerStylesFull();
    void testLayerStylesPartial();
    void testLayerStylesPartialVary();

    void testLayerStylesRects();
};

#endif /* __KIS_LAYER_STYLES_TEST_H */
