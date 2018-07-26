/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
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
