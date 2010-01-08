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

#include "kis_dyna_paintop_settings.h"

#include <kis_paint_action_type_option.h>

#include "kis_dyna_paintop_settings_widget.h"
#include "kis_dynaop_option.h"

KisDynaPaintOpSettings::KisDynaPaintOpSettings()
        : m_options(0)
{
}

bool KisDynaPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

qreal KisDynaPaintOpSettings::initWidth() const
{
    return m_options->m_dynaOption->initWidth();
}

qreal KisDynaPaintOpSettings::mass() const
{
    return m_options->m_dynaOption->mass();
}

qreal KisDynaPaintOpSettings::drag() const
{
    return m_options->m_dynaOption->drag();
}

bool KisDynaPaintOpSettings::useFixedAngle() const
{
    return m_options->m_dynaOption->useFixedAngle();
}

qreal KisDynaPaintOpSettings::xAngle() const
{
    return m_options->m_dynaOption->xAngle();
}

qreal KisDynaPaintOpSettings::yAngle() const
{
    return m_options->m_dynaOption->yAngle();
}

qreal KisDynaPaintOpSettings::widthRange() const
{
    return m_options->m_dynaOption->widthRange();
}

int KisDynaPaintOpSettings::action() const
{
    return m_options->m_dynaOption->action();
}

int KisDynaPaintOpSettings::circleRadius() const
{
    return m_options->m_dynaOption->circleRadius();
}

int KisDynaPaintOpSettings::lineCount() const
{
    return m_options->m_dynaOption->lineCount();
}

qreal KisDynaPaintOpSettings::lineSpacing() const
{
    return m_options->m_dynaOption->lineSpacing();
}


bool KisDynaPaintOpSettings::enableLine() const
{
    return m_options->m_dynaOption->enableLine();
}

bool KisDynaPaintOpSettings::twoCircles() const
{
    return m_options->m_dynaOption->twoCircles();
}

