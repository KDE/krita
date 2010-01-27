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

#ifndef KIS_BLUR_BENCHMARK_H
#define KIS_BLUR_BENCHMARK_H

#include <QtTest/QtTest>
#include <kis_types.h>
#include <KoColor.h>

class KisPaintDevice;
class KoColorSpace;
class KoColor;

class KisBlurBenchmark : public QObject
{
    Q_OBJECT
private:
    const KoColorSpace * m_colorSpace;
    KisPaintDeviceSP m_device;
    KoColor m_color;
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void benchmarkFilter();
    
};

#endif
