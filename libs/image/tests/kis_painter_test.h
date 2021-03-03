/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTER_TEST_H
#define KIS_PAINTER_TEST_H

#include <simpletest.h>

class KoColorSpace;

class KisPainterTest : public QObject
{
    Q_OBJECT

private:

    void allCsApplicator(void (KisPainterTest::* funcPtr)(const KoColorSpace*cs));
    void testSimpleBlt(const KoColorSpace * cs);
    void testPaintDeviceBltSelection(const KoColorSpace * cs);
    void testPaintDeviceBltSelectionIrregular(const KoColorSpace * cs);
    void testPaintDeviceBltSelectionInverted(const KoColorSpace * cs);

    void checkPerformance();


private Q_SLOTS:

    void testSimpleBlt();
    void testSelectionBltSelectionIrregular(); // Irregular selection
    void testPaintDeviceBltSelectionInverted(); // Inverted selection
    void testPaintDeviceBltSelectionIrregular(); // Irregular selection
    void testPaintDeviceBltSelection(); // Square selection
    void testSelectionBltSelection(); // Square selection
    void testSimpleAlphaCopy();
    void testSelectionBitBltFixedSelection();
    void testSelectionBitBltEraseCompositeOp();

    void testBitBltOldData();

    void testMassiveBltFixedSingleTile();
    void testMassiveBltFixedMultiTile();

    void testMassiveBltFixedMultiTileWithOpacity();

    void testMassiveBltFixedMultiTileWithSelection();

    void testMassiveBltFixedCornerCases();


    void testOptimizedCopying();
};

#endif

