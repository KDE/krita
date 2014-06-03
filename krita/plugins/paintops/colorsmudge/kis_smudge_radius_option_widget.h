/*
 *  Copyright (C) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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
#ifndef KIS_SMUDGE_RADIUS_OPTION_WIDGET_H
#define KIS_SMUDGE_RADIUS_OPTION_WIDGET_H

#include <kis_curve_option_widget.h>

class KisSmudgeRadiusOptionWidget: public KisCurveOptionWidget
{
public:
    KisSmudgeRadiusOptionWidget(const QString& label, const QString& sliderLabel, const QString& name, bool checked);
};
#endif // KIS_SMUDGE_RADIUS_OPTION_WIDGET_H
