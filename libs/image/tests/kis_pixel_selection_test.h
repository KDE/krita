/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PIXEL_SELECTION_H
#define KIS_PIXEL_SELECTION_H

#include <simpletest.h>

class KisPixelSelectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreation();
    void testSetSelected();
    void testSelect();
    void testInvert();
    void testClear();
    void testExtent();
    void testAddSelection();
    void testSubtractSelection();
    void testIntersectSelection();
    void testTotally();
    void testUpdateProjection();
    void testExactRectWithImage();
    void testUndo();
    void testInvertWithImage();
    void testCrossColorSpacePainting();
    void testOutlineCache();

    void testOutlineCacheTransactions();

    void testOutlineArtifacts();
};

#endif

