/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include <KoViewConverter.h>

#include "kis_sumi_paintop_settings.h"

#include "kis_sumi_paintop_settings_widget.h"
#include "kis_sumi_ink_option.h"
#include "kis_sumi_shape_option.h"

KisSumiPaintOpSettings::KisSumiPaintOpSettings()
        : m_options(0)
{
}

bool KisSumiPaintOpSettings::paintIncremental()
{
    return false;
}

QList<float> KisSumiPaintOpSettings::curve() const
{
    return m_options->m_sumiInkOption->curve();
}

int KisSumiPaintOpSettings::radius() const
{
    return m_options->m_sumiShapeOption->radius();
}

double KisSumiPaintOpSettings::sigma() const
{
    return m_options->m_sumiShapeOption->sigma();
}

bool KisSumiPaintOpSettings::mousePressure() const
{
    return m_options->m_sumiShapeOption->mousePressure();
}

int KisSumiPaintOpSettings::brushDimension() const
{
    return m_options->m_sumiShapeOption->brushDimension();
}

int KisSumiPaintOpSettings::inkAmount() const
{
    return m_options->m_sumiInkOption->inkAmount();
}

double KisSumiPaintOpSettings::shearFactor() const
{
    return m_options->m_sumiShapeOption->shearFactor();
}

double KisSumiPaintOpSettings::randomFactor() const
{
    return m_options->m_sumiShapeOption->randomFactor();
}

double KisSumiPaintOpSettings::scaleFactor() const
{
    return m_options->m_sumiShapeOption->scaleFactor();
}

bool KisSumiPaintOpSettings::useSaturation() const
{
    return m_options->m_sumiInkOption->useSaturation();
}

bool KisSumiPaintOpSettings::useOpacity() const
{
    return m_options->m_sumiInkOption->useOpacity();
}

bool KisSumiPaintOpSettings::useWeights() const
{
    return m_options->m_sumiInkOption->useWeights();
}

int KisSumiPaintOpSettings::pressureWeight() const
{
    return m_options->m_sumiInkOption->pressureWeight();
}

int KisSumiPaintOpSettings::bristleLengthWeight() const
{
    return m_options->m_sumiInkOption->bristleLengthWeight();
}

int KisSumiPaintOpSettings::bristleInkAmountWeight() const
{
    return m_options->m_sumiInkOption->bristleInkAmountWeight();
}

int KisSumiPaintOpSettings::inkDepletionWeight() const
{
    return m_options->m_sumiInkOption->inkDepletionWeight();
}


void KisSumiPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = radius() * 2 * scaleFactor() + 1;
    painter.setPen(Qt::black);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}


QRectF KisSumiPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = radius() * 2 * scaleFactor();
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}


void KisSumiPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        m_options->m_sumiShapeOption->setRadius( radius() + qRound(x) );
    }
    else // vice-versa
    {
        // we can do something different
    }

}
