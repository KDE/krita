/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LOW_MEMORY_BENCHMARK_H
#define __KIS_LOW_MEMORY_BENCHMARK_H

#include <QtTest>

class KisLowMemoryBenchmark : public QObject
{
    Q_OBJECT
private slots:
    void unlimitedMemoryNoHistoryNoPool();
    void unlimitedMemoryHistoryNoPool();
    void unlimitedMemoryHistoryPool50();

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
