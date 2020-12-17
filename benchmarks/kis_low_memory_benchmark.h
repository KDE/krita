/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LOW_MEMORY_BENCHMARK_H
#define __KIS_LOW_MEMORY_BENCHMARK_H

#include <QtTest>

class KisLowMemoryBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void unlimitedMemoryNoHistoryNoPool();
    void unlimitedMemoryHistoryNoPool();
    void unlimitedMemoryHistoryPool50();

    void memory2000History100Pool500HugeBrush();

private:
    void benchmarkWideArea(const QString presetFileName,
                           const QRectF &rect, qreal vstep,
                           int numCycles,
                           bool createTransaction,
                           int hardLimitMiB,
                           int softLimitMiB,
                           int poolLimitMiB,
                           int index);
};

#endif /* __KIS_LOW_MEMORY_BENCHMARK_H */
