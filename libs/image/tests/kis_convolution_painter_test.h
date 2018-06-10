/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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
};

#endif
