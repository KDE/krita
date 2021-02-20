/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CONVOLUTION_PAINTER_TEST_H
#define KIS_CONVOLUTION_PAINTER_TEST_H

#include <QtTest>
#include <kis_types.h>

class QBitArray;


class KisConvolutionPainterTest : public QObject
{
    Q_OBJECT

private:
    void testAsymmConvolutionImp(QBitArray channelFlags);
    void testGaussianBase(KisPaintDeviceSP dev, bool useFftw, const QString &prefix);
    void testGaussian(bool useFftw);
    void testGaussianSmall(bool useFftw);
    void testGaussianDetails(bool useFftw);
    void testNormalMap(KisPaintDeviceSP dev, bool useFftw, const QString &prefix);
    void testNormalMap(bool useFftw);

private Q_SLOTS:

    void testIdentityConvolution();
    void testSymmConvolution();

    void testAsymmAllChannels();
    void testAsymmSkipRed();
    void testAsymmSkipGreen();
    void testAsymmSkipBlue();
    void testAsymmSkipAlpha();

    void benchmarkConvolution();
    void testGaussianSpatial();
    void testGaussianFFTW();

    void testGaussianSmallSpatial();
    void testGaussianSmallFFTW();

    void testGaussianDetailsSpatial();
    void testGaussianDetailsFFTW();

    void testDilate();
    void testErode();

    void testNormalMapSpatial();
    void testNormalMapFFTW();
};

#endif
