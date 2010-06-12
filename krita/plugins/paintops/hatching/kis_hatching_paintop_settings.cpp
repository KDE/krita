/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hatching_paintop_settings.h"

#include <kis_paint_action_type_option.h>

KisHatchingPaintOpSettings::KisHatchingPaintOpSettings()
{
}

bool KisHatchingPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

void KisHatchingPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = radius() * 2 * + 1;
    painter.setPen(Qt::black);
    painter.drawEllipse(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos));
}


QRectF KisHatchingPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = radius() * 2;
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

int KisHatchingPaintOpSettings::proeba() const
{
    return 5;
}

int KisHatchingPaintOpSettings::radius() const
{
    return getInt("Hatching/radius");
}

bool KisHatchingPaintOpSettings::inkDepletion() const
{
    return getBool("Hatching/inkDepletion");
}


bool KisHatchingPaintOpSettings::opacity() const
{
    return getBool("Hatching/opacity");
}


bool KisHatchingPaintOpSettings::saturation() const
{
    return getBool("Hatching/saturation");
}


#if defined(HAVE_OPENGL)
QString KisHatchingPaintOpSettings::modelName() const
{
    return "3d-pencil";
}
#endif

