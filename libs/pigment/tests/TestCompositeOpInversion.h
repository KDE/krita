/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTCOMPOSITEOPINVERSION_H
#define TESTCOMPOSITEOPINVERSION_H

#include <QObject>

class TestCompositeOpInversion : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    void test();
    void test_data();

    void testColorPairSampler();
    void testColorPairSamplerRGB();

    void testF32ModesNaN();
    void testF32ModesNaN_data();

#if 0
    void testU16ModesConsistent();
    void testU16ModesConsistent_data();
#endif

    void testF32vsU16ConsistencyInSDR_data();
    void testF32vsU16ConsistencyInSDR();

    void testPreservesStrictSdrRange();
    void testPreservesStrictSdrRange_data();

    void testPreservesLooseSdrRange();
    void testPreservesLooseSdrRange_data();

    void testSrcCannotMakeNegative();
    void testSrcCannotMakeNegative_data();

    void testPreservesStrictNegative();
    void testPreservesStrictNegative_data();

    void testPreservesLooseNegative();
    void testPreservesLooseNegative_data();

    void testToneMappingPositive();
    void testToneMappingNegative();


    void dumpOpCategories();

private:
    /// Uncomment if you want to generate the sample
    /// sheets of every blendmode to verify their contiguity
    void generateSampleSheetsLong_data();
    void generateSampleSheetsLong();

private:
    // TODO: disabled for now
    void testF16Modes();
    void testF16Modes_data();

    /// just a simple test case to test exactly one color
    /// during debugging
    void testColor();

private:
    void testNegativeImpl(bool useStrictZeroCheck);
    void testPreservesSdrRangeImpl(bool useStrictRange);
};

#endif // TESTCOMPOSITEOPINVERSION_H
