/*
 * Copyright (c) 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#include "kis_grid_paintop_settings.h"
#include "kis_grid_paintop_settings_widget.h"
#include "kis_gridop_option.h"
#include "kis_grid_shape_option.h"
#include <kis_color_option.h>

KisGridPaintOpSettings::KisGridPaintOpSettings()
    : m_options(0)
{
}

KisPaintOpSettingsSP KisGridPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}


bool KisGridPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisGridPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML( elt );
    m_options->setConfiguration( this );
}

void KisGridPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}


int KisGridPaintOpSettings::divisionLevel() const
{
    return m_options->m_gridOption->divisionLevel();
}


int KisGridPaintOpSettings::gridWidth() const
{
    return m_options->m_gridOption->gridWidth();
}


int KisGridPaintOpSettings::gridHeight() const
{
    return m_options->m_gridOption->gridHeight();
}


bool KisGridPaintOpSettings::pressureDivision() const
{
    return m_options->m_gridOption->pressureDivision();
}


int KisGridPaintOpSettings::shape() const {
    return m_options->m_gridShapeOption->shape();
}


bool KisGridPaintOpSettings::useRandomOpacity() const
{
    return m_options->m_ColorOption->useRandomOpacity();
}


int KisGridPaintOpSettings::hue() const
{
    return m_options->m_ColorOption->hue();
}


int KisGridPaintOpSettings::saturation() const
{
    return m_options->m_ColorOption->saturation();
}

int KisGridPaintOpSettings::value() const
{
    return m_options->m_ColorOption->value();
}


bool KisGridPaintOpSettings::useRandomHSV() const
{
    return m_options->m_ColorOption->useRandomHSV();
}


bool KisGridPaintOpSettings::sampleInput() const
{
    return m_options->m_ColorOption->sampleInputColor();
}


qreal KisGridPaintOpSettings::horizBorder() const
{
    return m_options->m_gridOption->horizBorder();
}

qreal KisGridPaintOpSettings::vertBorder() const
{
    return m_options->m_gridOption->vertBorder();
}


bool KisGridPaintOpSettings::jitterBorder() const
{
    return m_options->m_gridOption->jitterBorder();
}


bool KisGridPaintOpSettings::colorPerParticle() const
{
    return m_options->m_ColorOption->colorPerParticle();
}



bool KisGridPaintOpSettings::fillBackground() const
{
    return m_options->m_ColorOption->fillBackground();
}



bool KisGridPaintOpSettings::mixBgColor() const
{
    return m_options->m_ColorOption->mixBgColor();
}



qreal KisGridPaintOpSettings::scale() const
{
    return m_options->m_gridOption->scale();
}


void KisGridPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal sizex = gridWidth() * scale();
    qreal sizey = gridHeight() * scale();

    painter.setPen(QColor(255,128,255));
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawRect(converter.documentToView(image->pixelToDocument(QRectF(0, 0, sizex, sizey).translated(- QPoint(sizex * 0.5, sizey * 0.5))).translated(pos)));
}


QRectF KisGridPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal sizex = gridWidth() * scale();
    qreal sizey = gridHeight() * scale();
    sizex += 2;
    sizey += 2;
    return image->pixelToDocument(QRectF(0, 0, sizex, sizey).translated(- QPoint(sizex * 0.5, sizey * 0.5))).translated(pos);
}



void KisGridPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    if (qAbs(x) > qAbs(y))
    {
        m_options->m_gridOption->setWidth( gridWidth() + qRound(x) );
        m_options->m_gridOption->setHeight( gridHeight() + qRound(x) );
    }else{
        //m_options->m_gridOption->setHeight( gridHeight() + qRound(y) );
    }
}
