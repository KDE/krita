/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_MYPAINT_PAINTOP_H_
#define KIS_MYPAINT_PAINTOP_H_

#include <QTime>

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>

class QPointF;
class KisPainter;

class MyPaintSettings;
class MyPaintSurface;
class MyPaintBrushResource;

class MyPaint : public KisPaintOp
{

public:
    MyPaint(const MyPaintSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~MyPaint();

    virtual bool incremental() const { return true; }

    qreal paintAt(const KisPaintInformation& info);
    qreal spacing(qreal & xSpacing, qreal & ySpacing, qreal pressure1, qreal pressure2) const;
    KisDistanceInformation paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, const KisDistanceInformation& savedDist);

private:
    MyPaintSurface* m_surface;
    const MyPaintSettings* m_settings;
    MyPaintBrushResource *m_brush;
    bool m_firstPoint;
};

#endif // KIS_MYPAINTPAINTOP_H_
