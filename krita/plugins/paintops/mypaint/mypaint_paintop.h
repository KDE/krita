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

class MyPaint : public KisPaintOp
{

public:
    MyPaint(const MyPaintSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~MyPaint();

    virtual bool incremental() const { return true; }

    void paintAt(const KisPaintInformation& info);
    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const;
    double paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist);

private:

    QTime m_eventTime;
    bool m_mypaintThinksStrokeHasEnded;
    MyPaintSurface* m_surface;
    const MyPaintSettings* m_settings;
};

#endif // KIS_MYPAINTPAINTOP_H_
