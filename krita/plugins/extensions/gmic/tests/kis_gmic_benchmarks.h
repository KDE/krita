/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_GMIC_BENCHMARKS_H_
#define _KIS_GMIC_BENCHMARKS_H_

#include <kis_types.h>

#include <QtTest>
#include <QImage>
#include <QColor>

#include <gmic.h>

class KisGmicBenchmarks : public QObject
{
    Q_OBJECT

private:
    gmic_image<float> m_gmicImage;
    QImage m_qImage;
    QColor m_darkOrange;
    KisPaintDeviceSP m_device;
    QRect m_rect;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testQImageConversion();
    void testKisPaintDeviceConversion();
    void testConversion();

    void testConvertToGmic();
    void testConvertFromGmic();

    void testConvertToGmicFast();
    void testConvertFromGmicFast();

};

#endif
