/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DATAMANAGER_BENCHMARK_H
#define KIS_DATAMANAGER_BENCHMARK_H

#include <QtTest>

class KisDatamanagerBenchmark : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase();
    void benchmarkCreation();
    void benchmarkWriteBytes();
    void benchmarkReadBytes();
    void benchmarkReadWriteBytes();
    void benchmarkReadWriteBytes2();
    void benchmarkExtent();
    void benchmarkClear();
    void benchmarkMemCpy();
};

#endif
