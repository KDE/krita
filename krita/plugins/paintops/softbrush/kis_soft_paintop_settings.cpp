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
#include "kis_soft_paintop_settings.h"

#include "kis_soft_paintop_settings_widget.h"
#include "kis_softop_option.h"

#include <kis_paint_action_type_option.h>

#include <KoViewConverter.h>

KisSoftPaintOpSettings::KisSoftPaintOpSettings()
    : m_options(0)
{
}

bool KisSoftPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}

int KisSoftPaintOpSettings::diameter() const
{
    return m_options->m_softOption->diameter();
}

qreal KisSoftPaintOpSettings::spacing() const
{
    return m_options->m_softOption->spacing();
}


qreal KisSoftPaintOpSettings::end() const
{
    return m_options->m_softOption->end();
}


qreal KisSoftPaintOpSettings::start() const
{
    return m_options->m_softOption->start();
}


qreal KisSoftPaintOpSettings::sigma() const
{
    return m_options->m_softOption->sigma();
}


void KisSoftPaintOpSettings::changePaintOpSize ( qreal x, qreal y ) const
{
    if (qAbs(x) > qAbs(y)){
        m_options->m_softOption->setDiameter( diameter() + qRound(x) );
    }
    else // vice-versa
    {
        // we can do something different
    }

}
void KisSoftPaintOpSettings::paintOutline ( const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, KisPaintOpSettings::OutlineMode _mode ) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = diameter();
    QRectF ellipseRect = converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos));
    QPen pen = painter.pen();
    // temporary solution til i find out the bug with RasterOp_XOR in OpenGL canvas
    pen.setColor(Qt::white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawEllipse(ellipseRect);
    painter.setPen(QColor(0,0,0,150));
    pen.setWidth(1);
    painter.drawEllipse(ellipseRect);
}

QRectF KisSoftPaintOpSettings::paintOutlineRect ( const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode ) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = diameter();
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}
