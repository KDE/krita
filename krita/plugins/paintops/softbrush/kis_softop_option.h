/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_SOFTOP_OPTION_H
#define KIS_SOFTOP_OPTION_H

#include <kis_softbrush_selection_widget.h>
#include <kis_paintop_option.h>
#include <krita_export.h>

/// 0 - CURVE , 1 - GAUSS
const QString SOFT_BRUSH_TIP = "Soft/brushTip";

const QString SOFT_END = "Soft/end";
const QString SOFT_START = "Soft/start";
const QString SOFT_SIGMA = "Soft/sigma";
const QString SOFT_SOFTNESS =  "Soft/softness";

const QString SOFTCURVE_CURVE = "SoftCurve/curve";
const QString SOFTCURVE_CONTROL_BY_PRESSURE = "SoftCurve/controlByPressure";

class KisSoftOpOption : public KisPaintOpOption
{
public:
    KisSoftOpOption();
    ~KisSoftOpOption();

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisSoftBrushSelectionWidget * m_options;
};

#endif
