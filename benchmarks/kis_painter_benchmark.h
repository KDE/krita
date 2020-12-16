/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTER_BENCHMARK_H
#define KIS_PAINTER_BENCHMARK_H

#include <QtTest>

#include <KoColor.h>

#include <kis_types.h>



class KisPainterBenchmark : public QObject
{
    Q_OBJECT
private:
    const KoColorSpace * m_colorSpace;
    KoColor m_color;
    QVector<QPointF> m_points;
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    
    void benchmarkBitBlt();
    void benchmarkFastBitBlt();
    void benchmarkBitBltSelection();
    void benchmarkFixedBitBlt();
    void benchmarkFixedBitBltSelection();
    
    void benchmarkDrawThickLine();
    void benchmarkDrawQtLine();
    void benchmarkDrawScanLine();

    void benchmarkBitBlt2();
    void benchmarkBitBltOldData();
    void benchmarkMassiveBltFixed();

    
};

#endif
