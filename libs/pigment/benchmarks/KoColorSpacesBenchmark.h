/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KO_COLOR_SPACES_BENCHMARK_H_
#define _KO_COLOR_SPACES_BENCHMARK_H_

#include <QObject>

class KoColorSpacesBenchmark : public QObject
{
    Q_OBJECT
private:
    void createRowsColumns();
private Q_SLOTS:
    void benchmarkAlpha_data();
    void benchmarkAlpha();
    void benchmarkAlpha2_data();
    void benchmarkAlpha2();
    void benchmarkSetAlpha_data();
    void benchmarkSetAlpha();
    void benchmarkSetAlpha2_data();
    void benchmarkSetAlpha2();
    void benchmarkSetAlphaIndividualCall_data();
    void benchmarkSetAlphaIndividualCall();
    void benchmarkSetAlpha2IndividualCall_data();
    void benchmarkSetAlpha2IndividualCall();
};

#endif
