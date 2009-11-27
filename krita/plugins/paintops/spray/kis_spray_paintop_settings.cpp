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

#include "kis_spray_paintop_settings.h"
#include "kis_sprayop_option.h"
#include "kis_spray_shape_option.h"

KisSprayPaintOpSettings::KisSprayPaintOpSettings()
        : m_options(0)
{
}

KisPaintOpSettingsSP KisSprayPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>(m_options->configuration());
    return settings;
}


bool KisSprayPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisSprayPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML(elt);
    m_options->setConfiguration(this);
}

void KisSprayPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML(doc, rootElt);
    delete settings;
}


int KisSprayPaintOpSettings::diameter() const
{
    return m_options->m_sprayOption->diameter();
}

qreal KisSprayPaintOpSettings::coverage() const
{
    return m_options->m_sprayOption->coverage();
}

qreal KisSprayPaintOpSettings::amount() const
{
    return m_options->m_sprayOption->amount();
}

qreal KisSprayPaintOpSettings::spacing() const
{
    return m_options->m_sprayOption->spacing();
}

qreal KisSprayPaintOpSettings::scale() const
{
    return m_options->m_sprayOption->scale();
}

bool KisSprayPaintOpSettings::jitterMovement() const
{
    return m_options->m_sprayOption->jitterMovement();
}

bool KisSprayPaintOpSettings::jitterSize() const
{
    return m_options->m_sprayOption->jitterSize();
}


int KisSprayPaintOpSettings::width() const
{
    return m_options->m_sprayShapeOption->width();
}


int KisSprayPaintOpSettings::height() const
{
    return m_options->m_sprayShapeOption->height();
}


int KisSprayPaintOpSettings::shape() const
{
    return m_options->m_sprayShapeOption->shape();
}

bool KisSprayPaintOpSettings::jitterShapeSize() const
{
    return m_options->m_sprayShapeOption->jitterShapeSize();
}

qreal KisSprayPaintOpSettings::heightPerc() const
{
    return m_options->m_sprayShapeOption->heightPerc();
}

bool KisSprayPaintOpSettings::proportional() const
{
    return m_options->m_sprayShapeOption->proportional();
}

qreal KisSprayPaintOpSettings::widthPerc() const
{
    return m_options->m_sprayShapeOption->widthPerc();
}

bool KisSprayPaintOpSettings::useDensity() const
{
    return m_options->m_sprayOption->useDensity();
}

int KisSprayPaintOpSettings::particleCount() const
{
    return m_options->m_sprayOption->particleCount();
}


bool KisSprayPaintOpSettings::useRandomOpacity() const
{
    return m_options->m_ColorOption->useRandomOpacity();
}


int KisSprayPaintOpSettings::hue() const
{
    return m_options->m_ColorOption->hue();
}


int KisSprayPaintOpSettings::saturation() const
{
    return m_options->m_ColorOption->saturation();
}

int KisSprayPaintOpSettings::value() const
{
    return m_options->m_ColorOption->value();
}


bool KisSprayPaintOpSettings::useRandomHSV() const
{
    return m_options->m_ColorOption->useRandomHSV();
}

QRectF KisSprayPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = diameter() * scale();
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

void KisSprayPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = diameter() * scale();
    painter.setPen(QColor(255,128,255));
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}


bool KisSprayPaintOpSettings::gaussian() const
{
    return m_options->m_sprayShapeOption->gaussian();
}


bool KisSprayPaintOpSettings::colorPerParticle() const
{
    return m_options->m_ColorOption->colorPerParticle();
}


bool KisSprayPaintOpSettings::fillBackground() const
{
    return m_options->m_ColorOption->fillBackground();
}


bool KisSprayPaintOpSettings::mixBgColor() const
{
    return m_options->m_ColorOption->mixBgColor();
}


bool KisSprayPaintOpSettings::sampleInput() const
{
    return m_options->m_ColorOption->sampleInputColor();
}


void KisSprayPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    if (qAbs(x) > qAbs(y)){
            // recoginze the left/right movement
            if (x > 0){
                m_options->m_sprayOption->setDiamter( diameter() + qRound(x) );
            }else{
                m_options->m_sprayOption->setDiamter( diameter() + qRound(x) );
            }
    }else{
    }
}


int KisSprayPaintOpSettings::fixedAngle() const
{
    return m_options->m_sprayShapeOption->fixedAngle();
}


bool KisSprayPaintOpSettings::fixedRotation() const
{
    return m_options->m_sprayShapeOption->fixedRotation();
}



qreal KisSprayPaintOpSettings::randomRotationWeight() const
{
    return m_options->m_sprayShapeOption->randomRotationWeight();
}


bool KisSprayPaintOpSettings::randomRotation() const
{
    return m_options->m_sprayShapeOption->randomRotation();
}



bool KisSprayPaintOpSettings::followCursor() const
{
    return m_options->m_sprayShapeOption->followCursor();
}


qreal KisSprayPaintOpSettings::followCursorWeigth() const
{
        return m_options->m_sprayShapeOption->followCursorWeigth();
}



QImage KisSprayPaintOpSettings::image() const
{
    return m_options->m_sprayShapeOption->image();
}
