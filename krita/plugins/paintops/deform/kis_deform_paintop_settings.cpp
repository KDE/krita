/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <KoViewConverter.h>

#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

int KisDeformPaintOpSettings::radius() const
{
    return getInt("Deform/radius");
}


double KisDeformPaintOpSettings::deformAmount() const
{
    return getDouble("Deform/deformAmount");
}

bool KisDeformPaintOpSettings::bilinear() const
{
    return getBool("Deform/bilinear");
}

bool KisDeformPaintOpSettings::useMovementPaint() const
{
    return getBool("Deform/useMovementPaint");
}

bool KisDeformPaintOpSettings::useCounter() const
{
    return getBool("Deform/useCounter");
}

bool KisDeformPaintOpSettings::useOldData() const
{
    return getBool("Deform/useOldData");
}

int KisDeformPaintOpSettings::deformAction() const
{
    return getInt("Deform/deformAction");
}

qreal KisDeformPaintOpSettings::spacing() const
{
    return getDouble("Deform/spacing");
}

QRectF KisDeformPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = radius() * 2;
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

void KisDeformPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = radius() * 2;

#if 0
//     painter.setPen( QColor(128,255,128) );
//     painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRectF sizerc = converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos));
    QPen pen = painter.pen();
    pen.setColor(QColor(3, 3, 3, 150));
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(sizerc);
    pen.setColor(Qt::white);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawEllipse(sizerc);
#else
    painter.setPen(Qt::black);
#endif
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}