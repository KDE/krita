/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QPainter>

#include <KoViewConverter.h>
#include "kis_image.h"

#include "kis_sumi_paintop_settings.h"
#include "kis_sumi_shape_option.h"

bool KisSumiPaintOpSettings::paintIncremental()
{
    return false;
}


void KisSumiPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = getInt(SUMI_RADIUS) * 2 * getDouble(SUMI_SCALE) + 1;
    painter.setPen(Qt::black);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}


QRectF KisSumiPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = getInt(SUMI_RADIUS) * 2 * getDouble(SUMI_SCALE) + 1;
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}


