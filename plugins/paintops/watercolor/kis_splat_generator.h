/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SPLAT_GENERATOR
#define KIS_SPLAT_GENERATOR

#include <QVector>
#include "kis_wetmap.h"
#include "kis_splat.h"

#include "KoColor.h"
#include "kis_types.h"

class SplatGenerator
{
public:
    SplatGenerator(int width, KoColor &clr, KisPaintDeviceSP dev);

    void generateFromPoints(QVector<QPointF> &points, int msec);

private:
    QVector<KisSplat*> *m_flowing;
    QVector<KisSplat*> *m_fixed;
    QVector<KisSplat*> *m_dried;

    KisWetMap *m_wetMap;

    int m_width;
    KoColor m_color;
    KisPaintDeviceSP m_device;
};

#endif
