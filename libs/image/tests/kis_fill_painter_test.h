/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILL_PAINTER_TEST_H
#define KIS_FILL_PAINTER_TEST_H

#include <simpletest.h>

class KisFillPainterTest : public QObject
{
    Q_OBJECT

private:
    void benchmarkFillPainter(const QPoint &startPoint, bool useCompositioning);

private Q_SLOTS:

    void testCreation();
    void benchmarkFillPainter();
    void benchmarkFillPainterOffset();
    void benchmarkFillPainterOffsetCompositioning();
    void benchmarkFillingScanlineColor();
    void benchmarkFillingScanlineSelection();

    void testPatternFill();
};

#endif
