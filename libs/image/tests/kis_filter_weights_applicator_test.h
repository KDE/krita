/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FILTER_WEIGHTS_APPLICATOR_TEST_H
#define __KIS_FILTER_WEIGHTS_APPLICATOR_TEST_H

#include <QtTest>

class KisFilterWeightsApplicatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSpan_Scale_2_0_Aligned();
    void testSpan_Scale_2_0_Shift_0_5();
    void testSpan_Scale_2_0_Shift_0_75();

    void testSpan_Scale_0_5_Aligned();
    void testSpan_Scale_0_5_Shift_0_5();
    void testSpan_Scale_0_5_Shift_0_25();
    void testSpan_Scale_0_5_Shift_0_375();
    void testSpan_Scale_0_5_Shift_m0_5();
    void testSpan_Scale_0_5_Shift_m0_25();
    void testSpan_Scale_0_5_Shift_m0_375();

    void testSpan_Scale_1_0_Aligned_Mirrored();
    void testSpan_Scale_0_5_Aligned_Mirrored();
    void testSpan_Scale_0_5_Shift_0_125_Mirrored();

    void testProcessLine_Scale_1_0_Aligned();
    void testProcessLine_Scale_1_0_Shift_0_5();
    void testProcessLine_Scale_1_0_Shift_m0_5();
    void testProcessLine_Scale_1_0_Shift_0_25();
    void testProcessLine_Scale_1_0_Shift_m0_25();

    void testProcessLine_Scale_0_5_Aligned();
    void testProcessLine_Scale_0_5_Shift_0_25();

    void testProcessLine_Scale_2_0_Aligned();
    void testProcessLine_Scale_2_0_Shift_0_25();
    void testProcessLine_Scale_2_0_Shift_0_5();

    void testProcessLine_Scale_1_0_Aligned_Clamped();
    void testProcessLine_Scale_0_5_Aligned_Clamped();
    void testProcessLine_Scale_2_0_Aligned_Clamped();

    void testProcessLine_Scale_1_0_Aligned_Mirrored();
    void testProcessLine_Scale_1_0_Shift_0_25_Mirrored();

    void testProcessLine_Scale_0_5_Aligned_Mirrored_Clamped();
    void testProcessLine_Scale_0_5_Shift_0_125_Mirrored();

    void testProcessLine_NearestNeighbourFilter_2x();
    void testProcessLine_NearestNeighbourFilter_1x();
    void testProcessLine_NearestNeighbourFilter_05x();
    void testProcessLine_NearestNeighbourFilter_077x();
    void testProcessLine_NearestNeighbourFilter_074x();
    void testProcessLine_NearestNeighbourFilter_075x();
    void testProcessLine_NearestNeighbourFilter_15x();
    void testProcessLine_NearestNeighbourFilter_0098x_horizontal();
    void testProcessLine_NearestNeighbourFilter_0098x_vertical();

    void benchmarkProcesssLine();
};

#endif /* __KIS_FILTER_WEIGHTS_APPLICATOR_TEST_H */
