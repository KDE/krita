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

#ifndef KIS_HLINEITERATOR_BENCHMARK_H
#define KIS_HLINEITERATOR_BENCHMARK_H

#include <QtTest/QtTest>

class KisPaintDevice;
class KoColorSpace;
class KoColor;


class KisRandomIteratorBenchmark : public QObject
{
    Q_OBJECT

private:
    const KoColorSpace * m_colorSpace;
    KisPaintDevice * m_device;        
    KoColor * m_color;
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void benchmarkCreation();
    
    // memcpy from KoColor to device
    void benchmarkWriteBytes();
    // memcpy from device to KoColor
    void benchmarkReadBytes();
    // const vline iterator used
    void benchmarkConstReadBytes();
    // copy from one device to another
    void benchmarkReadWriteBytes();
    // randomly copy data
    void benchmarkTotalRandom();
    // randomly copy data
    void benchmarkTotalRandomConst();
    // tile by tile benchmark
    void benchmarkTileByTileWrite();
    
    void benchmarkNoMemCpy();
    void benchmarkConstNoMemCpy();
    void benchmarkTwoIteratorsNoMemCpy();
};

#endif
