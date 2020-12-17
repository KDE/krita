/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MARKER_PAINTER_TEST_H
#define __KIS_MARKER_PAINTER_TEST_H

#include <QtTest/QtTest>

class KisMarkerPainterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testFillHalfBrushDiff();
    void testFillFullCircle();
    void testFillCirclesDiffSingle();
    void testFillCirclesDiff();
};

#endif /* __KIS_MARKER_PAINTER_TEST_H */
