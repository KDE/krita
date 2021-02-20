/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_PAINTER_TEST_H
#define KIS_GRADIENT_PAINTER_TEST_H

#include <QtTest>

class KisGradientPainterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSimplifyPath();

    void testShapedGradientPainterRect();
    void testShapedGradientPainterRectPierced();
    void testShapedGradientPainterNonRegular();
    void testShapedGradientPainterNonRegularPierced();

    void testFindShapedExtremums();
    void testSplitDisjointPaths();

    void testCachedStrategy();
};

#endif
