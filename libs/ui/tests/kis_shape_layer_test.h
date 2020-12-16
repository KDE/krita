/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSHAPELAYERTEST_H
#define KISSHAPELAYERTEST_H

#include <QObject>

class KisShapeLayerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testMergeDown();
    void testScaleAndMergeDown();
    void testMergingShapeZIndexes();

    void testCloneScaledLayer();
};

#endif // KISSHAPELAYERTEST_H
