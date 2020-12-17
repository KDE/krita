/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ITERATOR_BENCHMARK_H
#define KIS_ITERATOR_BENCHMARK_H

#include <QtTest>

class KoColorSpace;

class KisIteratorBenchmark : public QObject
{
    Q_OBJECT

private:
    void allCsApplicator(void (KisIteratorBenchmark::* funcPtr)(const KoColorSpace*cs));

    void vLineIterNG(const KoColorSpace * cs);
    template <bool useXY>
    void sequentialIter(const KoColorSpace * colorSpace);
    void hLineIterNG(const KoColorSpace * cs);
    void randomAccessor(const KoColorSpace * cs);


private Q_SLOTS:

    void runBenchmark();
};

#endif

