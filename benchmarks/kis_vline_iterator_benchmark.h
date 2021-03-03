/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_VLINE_ITERATOR_BENCHMARK_H
#define KIS_VLINE_ITERATOR_BENCHMARK_H

#include <simpletest.h>

class KisPaintDevice;
class KoColor;
class KoColorSpace;


class KisVLineIteratorBenchmark : public QObject
{
    Q_OBJECT

private:
    const KoColorSpace * m_colorSpace;
    KisPaintDevice * m_device;        
    KoColor * m_color;
private Q_SLOTS:
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
    
    void benchmarkNoMemCpy();
    void benchmarkConstNoMemCpy();
    void benchmarkTwoIteratorsNoMemCpy();
};

#endif
