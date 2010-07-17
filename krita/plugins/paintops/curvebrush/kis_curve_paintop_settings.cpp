/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_curve_paintop_settings.h>
#include "kis_image.h"

KisCurvePaintOpSettings::KisCurvePaintOpSettings()
{
}

bool KisCurvePaintOpSettings::paintIncremental()
{
    return false;
}

QRectF KisCurvePaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    Q_UNUSED(_mode);
    QRectF rect = QRectF(-5, -5, 10, 10);
    return image->pixelToDocument(rect).translated(pos);
}

void KisCurvePaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CursorIsOutline) return;
    QRectF rect2 = paintOutlineRect(pos, image, _mode);
    painter.drawLine(rect2.topLeft(), rect2.bottomRight());
    painter.drawLine(rect2.topRight(), rect2.bottomLeft());
}

