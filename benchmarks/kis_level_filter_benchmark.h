/*
 *  SPDX-FileCopyrightText: 2015 Thorsten Zachmann <zachmann@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVEL_FILTER_BENCHMARK_H
#define KIS_LEVEL_FILTER_BENCHMARK_H

#include <simpletest.h>
#include <kis_types.h>
#include <KoColor.h>
#include <kis_paint_device.h>

class KoColor;

class KisLevelFilterBenchmark : public QObject
{
    Q_OBJECT

private:
    const KoColorSpace * m_colorSpace;
    KisPaintDeviceSP m_device;
    KoColor m_color;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void benchmarkFilter();
};

#endif // KIS_LEVEL_FILTER_BENCHMARK_H
