/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROJECTION_BENCHMARK_H
#define KIS_PROJECTION_BENCHMARK_H

#include <simpletest.h>

/// loads the image, computes the projection and saves it to another file
class KisProjectionBenchmark : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void benchmarkProjection();
    void benchmarkLoading();
};

#endif
