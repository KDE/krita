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

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_spray_paintop_settings.h"
#include "kis_sprayop_option.h"
#include "kis_spray_shape_option.h"


bool KisSprayPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

int KisSprayPaintOpSettings::diameter() const
{ 
    return getInt("Spray/diameter");
}

qreal KisSprayPaintOpSettings::aspect() const
{
    return getDouble("Spray/aspect");
}

qreal KisSprayPaintOpSettings::coverage() const
{
    return getDouble("Spray/coverage");
}

qreal KisSprayPaintOpSettings::amount() const
{
    return getDouble("Spray/amount");
}

qreal KisSprayPaintOpSettings::spacing() const
{
    return getDouble("Spray/spacing");
}

qreal KisSprayPaintOpSettings::scale() const
{
    return getDouble("Spray/aspect");
}


qreal KisSprayPaintOpSettings::brushRotation() const
{
    return getDouble("Spray/rotation");
}


bool KisSprayPaintOpSettings::jitterMovement() const
{
    return getBool("Spray/jitterMovement");
}

int KisSprayPaintOpSettings::width() const{
    return getInt(SPRAYSHAPE_WIDTH);
}


int KisSprayPaintOpSettings::height() const
{
    return getInt(SPRAYSHAPE_HEIGHT);
}


int KisSprayPaintOpSettings::shape() const
{
    return getInt(SPRAYSHAPE_SHAPE);
}

bool KisSprayPaintOpSettings::randomSize() const
{
    return getBool(SPRAYSHAPE_RANDOM_SIZE);
}

bool KisSprayPaintOpSettings::proportional() const
{
    return getBool(SPRAYSHAPE_PROPORTIONAL);
}

int KisSprayPaintOpSettings::fixedAngle() const
{
    return getInt(SPRAYSHAPE_FIXED_ANGEL);
}


bool KisSprayPaintOpSettings::fixedRotation() const
{
    return getBool(SPRAYSHAPE_FIXED_ROTATION);
}


qreal KisSprayPaintOpSettings::randomRotationWeight() const
{
    return getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT);
}


bool KisSprayPaintOpSettings::randomRotation() const
{
    return getBool(SPRAYSHAPE_RANDOM_ROTATION);
}



bool KisSprayPaintOpSettings::followCursor() const
{
    return getBool(SPRAYSHAPE_FOLLOW_CURSOR);
}


qreal KisSprayPaintOpSettings::followCursorWeigth() const
{
    return getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT);
}


bool KisSprayPaintOpSettings::useDensity() const
{
    return getBool("Spray/useDensity");
}

int KisSprayPaintOpSettings::particleCount() const
{
    return getInt("Spray/particleCount");
}


bool KisSprayPaintOpSettings::useRandomOpacity() const
{
    return getBool("ColorOption/useRandomOpacity");
}


int KisSprayPaintOpSettings::hue() const
{
    return getInt("ColorOption/hue");
}


int KisSprayPaintOpSettings::saturation() const
{
    return getInt("ColorOption/saturation");
}

int KisSprayPaintOpSettings::value() const
{
    return getInt("ColorOption/value");
}


bool KisSprayPaintOpSettings::useRandomHSV() const
{
    return getBool("ColorOption/useRandomHSV");
}

QRectF KisSprayPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal width = diameter() * scale();
    qreal height = diameter() * aspect() * scale();
    width += 10;
    height += 10;
    QRectF rc = QRectF(0, 0, width, height);
    return image->pixelToDocument(rc.translated(- QPoint(width * 0.5, height * 0.5))).translated(pos);
}

void KisSprayPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal width = diameter() * scale();
    qreal height = diameter() * aspect() * scale();
    painter.setPen(QColor(255,128,255));
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, width, height).translated(- QPoint(width * 0.5, height * 0.5))).translated(pos)));
}


bool KisSprayPaintOpSettings::gaussian() const
{
    return getBool("Spray/gaussian");
}


bool KisSprayPaintOpSettings::colorPerParticle() const
{
    return getBool("ColorOption/colorPerParticle");
}


bool KisSprayPaintOpSettings::fillBackground() const
{
    return getBool("ColorOption/fillBackground");
}


bool KisSprayPaintOpSettings::mixBgColor() const
{
    return getBool("ColorOption/mixBgColor");
}


bool KisSprayPaintOpSettings::sampleInput() const
{
    return getBool("ColorOption/sampleInputColor");
}



QImage KisSprayPaintOpSettings::image() const
{
    return m_image;
}

