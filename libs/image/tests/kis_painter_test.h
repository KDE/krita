/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PAINTER_TEST_H
#define KIS_PAINTER_TEST_H

#include <QtTest>

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

