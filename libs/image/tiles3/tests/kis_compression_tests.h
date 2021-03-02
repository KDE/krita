/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COMPRESSION_TESTS_H
#define KIS_COMPRESSION_TESTS_H

#include <simpletest.h>

class KisAbstractCompression;

class KisCompressionTests : public QObject
{
    Q_OBJECT

private:
    void roundTrip(KisAbstractCompression *compression);
    void roundTripTwoPass(KisAbstractCompression *compression);

    void benchmarkCompression(KisAbstractCompression *compression);
    void benchmarkCompressionTwoPass(KisAbstractCompression *compression);

    void benchmarkDecompression(KisAbstractCompression *compression);
    void benchmarkDecompressionTwoPass(KisAbstractCompression *compression);

    void testOverflow(KisAbstractCompression *compression);

private Q_SLOTS:
    void testLzfRoundTrip();
    void testLzfOverflow();

    void benchmarkMemCpy();

    void benchmarkCompressionLzf();
    void benchmarkCompressionLzfTwoPass();
    void benchmarkDecompressionLzf();
    void benchmarkDecompressionLzfTwoPass();
};

#endif /* KIS_COMPRESSION_TESTS_H */

