/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_FAST_MATH_BENCHMARK_H_
#define _KIS_FAST_MATH_BENCHMARK_H_

#include <simpletest.h>

class KisFastMathBenchmark : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void benchmarkFastAtan2();
    void benchmarkLibCAtan2();
};

#endif
