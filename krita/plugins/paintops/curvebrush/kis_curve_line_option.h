/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_CURVE_LINE_OPTION_H
#define KIS_CURVE_LINE_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisCurveOpOptionsWidget;

// new rewrite
const QString CURVE_LINE_WIDTH = "Curve/lineWidth"; // same as in sketch
const QString CURVE_PAINT_CONNECTION_LINE = "Curve/makeConnection"; // same as in sketch
const QString CURVE_STROKE_HISTORY_SIZE = "Curve/strokeHistorySize";
const QString CURVE_SMOOTHING = "Curve/smoothing";
const QString CURVE_CURVES_OPACITY = "Curve/curvesOpacity";

class KisCurveOpOption : public KisPaintOpOption
{
public:
    KisCurveOpOption();
    ~KisCurveOpOption();

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisCurveOpOptionsWidget * m_options;

};

class CurveProperties{
public:
    int lineWidth;
    int historySize;
    qreal curvesOpacity;
    bool paintConnectionLine;
    bool smoothing;

    void readOptionSetting(const KisPropertiesConfiguration* settings) {
        lineWidth = settings->getInt(CURVE_LINE_WIDTH);
        historySize = settings->getInt(CURVE_STROKE_HISTORY_SIZE);
        paintConnectionLine = settings->getBool(CURVE_PAINT_CONNECTION_LINE);
        smoothing = settings->getBool(CURVE_SMOOTHING);
        curvesOpacity = settings->getDouble(CURVE_CURVES_OPACITY);
    }
};

#endif
