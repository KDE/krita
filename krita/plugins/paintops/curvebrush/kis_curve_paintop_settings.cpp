/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_curve_paintop_settings.h>

KisCurvePaintOpSettings::KisCurvePaintOpSettings()
{
}

bool KisCurvePaintOpSettings::paintIncremental()
{
    return false;
}

int KisCurvePaintOpSettings::minimalDistance() const
{
    return getInt("min_distance");
}

int KisCurvePaintOpSettings::curveAction() const
{
    return getInt("curve_action");
}

int KisCurvePaintOpSettings::interval() const
{
    return getInt("interval");
}

