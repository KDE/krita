/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BLUR_BENCHMARK_H
#define KIS_BLUR_BENCHMARK_H

#include <QtTest>
#include <kis_types.h>
#include <KoColor.h>
#include <kis_paint_device.h>

class KoColor;

class KisBlurBenchmark : public QObject
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

#endif
