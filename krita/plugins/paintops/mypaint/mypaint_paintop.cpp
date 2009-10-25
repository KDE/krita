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

#include "mypaint_paintop.h"
#include "mypaint_paintop_settings.h"

#include <cmath>

#include <QRect>
#include <QList>
#include <QColor>

#include <qdebug.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoInputDevice.h>
#include <KoCompositeOp.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>
#include <kis_vec.h>
#include <kis_datamanager.h>
#include <kis_paint_information.h>

MyPaint::MyPaint(const MyPaintSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
{
    Q_ASSERT(settings);
}

MyPaint::~MyPaint()
{
}

double MyPaint::spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        return 1.0;
}


void MyPaint::paintAt(const KisPaintInformation& info)
{
}

double MyPaint::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist)
{
    Q_UNUSED(savedDist);

    if (!painter()) return 0;

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return  dragVec.norm();
}
