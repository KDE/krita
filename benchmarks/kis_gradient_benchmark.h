/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_BENCHMARK_H
#define KIS_GRADIENT_BENCHMARK_H

#include <KoColor.h>

#include <kis_types.h>
#include <QtTest>
#include <kis_paint_device.h>

class KoColor;

class KisGradientBenchmark : public QObject
{
    Q_OBJECT
private:
    const KoColorSpace * m_colorSpace;
    KoColor m_color;
    KisPaintDeviceSP m_device;        
    int m_startX;
    int m_startY;
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    
    void benchmarkGradient();
    
    
    
};

#endif
