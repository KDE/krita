/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void testProcessLine_NearestNeighbourFilter_051x();
    void testProcessLine_NearestNeighbourFilter_15x();
    void testProcessLine_NearestNeighbourFilter_all();
    void testProcessLine_NearestNeighbourFilter_0098x_horizontal();
    void testProcessLine_NearestNeighbourFilter_0098x_vertical();

    void benchmarkProcesssLine();
};

#endif /* __KIS_FILTER_WEIGHTS_APPLICATOR_TEST_H */
