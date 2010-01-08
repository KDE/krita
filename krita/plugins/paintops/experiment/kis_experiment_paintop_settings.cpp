
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

#include <KoViewConverter.h>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_experiment_paintop_settings.h"
#include "kis_experimentop_option.h"
#include "kis_experiment_shape_option.h"

KisExperimentPaintOpSettings::KisExperimentPaintOpSettings()
        : m_options(0)
{
}

bool KisExperimentPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

int KisExperimentPaintOpSettings::startSize() const
{
    return m_options->m_experimentOption->startSize();
}



int KisExperimentPaintOpSettings::endSize() const
{
    return m_options->m_experimentOption->endSize();
}


qreal KisExperimentPaintOpSettings::spacing() const
{
    return m_options->m_experimentOption->spacing();
}

qreal KisExperimentPaintOpSettings::scale() const
{
    return m_options->m_experimentOption->scale();
}




bool KisExperimentPaintOpSettings::jitterMovement() const
{
    return m_options->m_experimentOption->jitterMovement();
}

int KisExperimentPaintOpSettings::width() const
{
    return m_options->m_experimentShapeOption->width();
}


int KisExperimentPaintOpSettings::height() const
{
    return m_options->m_experimentShapeOption->height();
}


int KisExperimentPaintOpSettings::shape() const
{
    return m_options->m_experimentShapeOption->shape();
}

bool KisExperimentPaintOpSettings::jitterShapeSize() const
{
    return m_options->m_experimentShapeOption->jitterShapeSize();
}

bool KisExperimentPaintOpSettings::proportional() const
{
    return m_options->m_experimentShapeOption->proportional();
}


bool KisExperimentPaintOpSettings::useRandomOpacity() const
{
    return m_options->m_ColorOption->useRandomOpacity();
}


int KisExperimentPaintOpSettings::hue() const
{
    return m_options->m_ColorOption->hue();
}


int KisExperimentPaintOpSettings::saturation() const
{
    return m_options->m_ColorOption->saturation();
}

int KisExperimentPaintOpSettings::value() const
{
    return m_options->m_ColorOption->value();
}


bool KisExperimentPaintOpSettings::useRandomHSV() const
{
    return m_options->m_ColorOption->useRandomHSV();
}

QRectF KisExperimentPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal width = startSize() * scale();
    qreal height = startSize() * scale();
    width += 10;
    height += 10;
    QRectF rc = QRectF(0, 0, width, height);
    return image->pixelToDocument(rc.translated(- QPoint(width * 0.5, height * 0.5))).translated(pos);
}

void KisExperimentPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal width = startSize() * scale();
    qreal height = startSize() * scale();
    painter.setPen(QColor(255,128,255));
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, width, height).translated(- QPoint(width * 0.5, height * 0.5))).translated(pos)));
}


bool KisExperimentPaintOpSettings::colorPerParticle() const
{
    return m_options->m_ColorOption->colorPerParticle();
}


bool KisExperimentPaintOpSettings::fillBackground() const
{
    return m_options->m_ColorOption->fillBackground();
}


bool KisExperimentPaintOpSettings::mixBgColor() const
{
    return m_options->m_ColorOption->mixBgColor();
}


bool KisExperimentPaintOpSettings::sampleInput() const
{
    return m_options->m_ColorOption->sampleInputColor();
}


void KisExperimentPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    if (qAbs(x) > qAbs(y)){
            // recoginze the left/right movement
            if (x > 0){
                m_options->m_experimentOption->setDiameter( startSize() + qRound(x) );
            }else{
                m_options->m_experimentOption->setDiameter( startSize() + qRound(x) );
            }
    }else{
    }
}


int KisExperimentPaintOpSettings::fixedAngle() const
{
    return m_options->m_experimentShapeOption->fixedAngle();
}


bool KisExperimentPaintOpSettings::fixedRotation() const
{
    return m_options->m_experimentShapeOption->fixedRotation();
}



qreal KisExperimentPaintOpSettings::randomRotationWeight() const
{
    return m_options->m_experimentShapeOption->randomRotationWeight();
}


bool KisExperimentPaintOpSettings::randomRotation() const
{
    return m_options->m_experimentShapeOption->randomRotation();
}



bool KisExperimentPaintOpSettings::followCursor() const
{
    return m_options->m_experimentShapeOption->followCursor();
}


qreal KisExperimentPaintOpSettings::followCursorWeigth() const
{
        return m_options->m_experimentShapeOption->followCursorWeigth();
}



QImage KisExperimentPaintOpSettings::image() const
{
    return m_options->m_experimentShapeOption->image();
}
