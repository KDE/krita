/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_COMPRESSORS_TEST_H
#define KIS_TILE_COMPRESSORS_TEST_H

#include <QtTest>

class KisAbstractTileCompressor;

class KisTileCompressorsTest : public QObject
{
    Q_OBJECT
private:
    void doRoundTrip(KisAbstractTileCompressor *compressor);
    void doLowLevelRoundTrip(KisAbstractTileCompressor *compressor);
    void doLowLevelRoundTripIncompressible(KisAbstractTileCompressor *compressor);


private Q_SLOTS:
    void testRoundTripLegacy();
    void testLowLevelRoundTripLegacy();

    void testRoundTrip2();
    void testLowLevelRoundTrip2();
    void testLowLevelRoundTripIncompressible2();
};

#endif /* KIS_TILE_COMPRESSORS_TEST_H */

