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

#include "kis_grid_paintop_settings.h"
#include "kis_grid_paintop_settings_widget.h"
#include "kis_gridop_option.h"
#include "kis_grid_shape_option.h"
#include "kis_grid_color_option.h"

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


int KisGridPaintOpSettings::width() const {
    return m_options->m_gridShapeOption->width();
}


int KisGridPaintOpSettings::height() const {
    return m_options->m_gridShapeOption->height();
}

int KisGridPaintOpSettings::object() const {
    return m_options->m_gridShapeOption->object();
}


int KisGridPaintOpSettings::shape() const {
    return m_options->m_gridShapeOption->shape();
}

bool KisGridPaintOpSettings::jitterShapeSize() const {
    return m_options->m_gridShapeOption->jitterShapeSize();
}

qreal KisGridPaintOpSettings::heightPerc() const {
    return m_options->m_gridShapeOption->heightPerc();
}

bool KisGridPaintOpSettings::proportional() const {
    return m_options->m_gridShapeOption->proportional();
}

qreal KisGridPaintOpSettings::widthPerc() const {
    return m_options->m_gridShapeOption->widthPerc();
}



qreal KisGridPaintOpSettings::maxTresh() const
{
    return m_options->m_gridShapeOption->maxTresh();
}


qreal KisGridPaintOpSettings::minTresh() const
{
    return m_options->m_gridShapeOption->minTresh();
}


bool KisGridPaintOpSettings::highRendering() const
{
    return m_options->m_gridShapeOption->highRendering();
}



bool KisGridPaintOpSettings::useRandomOpacity() const
{
    return m_options->m_gridColorOption->useRandomOpacity();
}


int KisGridPaintOpSettings::hue() const
{
    return m_options->m_gridColorOption->hue();
}


int KisGridPaintOpSettings::saturation() const
{
    return m_options->m_gridColorOption->saturation();
}

int KisGridPaintOpSettings::value() const
{
    return m_options->m_gridColorOption->value();
}


bool KisGridPaintOpSettings::useRandomHSV() const
{
    return m_options->m_gridColorOption->useRandomHSV();
}

bool KisGridPaintOpSettings::gaussian() const
{
    return m_options->m_gridShapeOption->gaussian();
}

bool KisGridPaintOpSettings::sampleInput() const
{
    return m_options->m_gridColorOption->sampleInputColor();
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
    return m_options->m_gridColorOption->colorPerParticle();
}



bool KisGridPaintOpSettings::fillBackground() const
{
    return m_options->m_gridColorOption->fillBackground();
}


qreal KisGridPaintOpSettings::scale() const
{
    return m_options->m_gridOption->scale();
}
