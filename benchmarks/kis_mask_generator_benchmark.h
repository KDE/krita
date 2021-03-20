/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MASK_GENERATOR_BENCHMARK_H
#define KIS_MASK_GENERATOR_BENCHMARK_H

#include <simpletest.h>

class KisMaskGeneratorBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void benchmarkCircle();
    void benchmarkSIMD_SharpBrush();
    void benchmarkSIMD_FadedBrush();
    void benchmarkSquare();

};

#endif
