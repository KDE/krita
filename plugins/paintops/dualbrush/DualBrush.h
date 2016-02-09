/*
  *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef DUALBRUSH_BRUSH_H_
#define DUALBRUSH_BRUSH_H_

#include <QVector>

#include <KoColor.h>

#include "DualBrushOption.h"
#include "kis_paint_device.h"
#include <brushengine/kis_random_source.h>


class DualBrushBrush
{

public:
    DualBrushBrush(const DualBrushProperties * properties, KoColorTransformation* transformation);
    ~DualBrushBrush();
    void paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color, qreal additionalScale);

private:
    KoColor m_inkColor;
    int m_counter;
    const DualBrushProperties * m_properties;
    KoColorTransformation* m_transfo;
    int m_saturationId;
    KisRandomSource m_randomSource;
};

#endif
