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
    ~KisCurveOpOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisCurveOpOptionsWidget * m_options;

};

class KisCurveOptionProperties : public KisPaintopPropertiesBase
{
public:
    bool curve_paint_connection_line;
    bool curve_smoothing;
    int curve_stroke_history_size;
    int curve_line_width;
    qreal curve_curves_opacity;

    void readOptionSettingImpl(const KisPropertiesConfiguration *config) override {
        curve_paint_connection_line = config->getBool(CURVE_PAINT_CONNECTION_LINE);
        curve_smoothing = config->getBool(CURVE_SMOOTHING);
        curve_stroke_history_size = config->getInt(CURVE_STROKE_HISTORY_SIZE);
        curve_line_width = config->getInt(CURVE_LINE_WIDTH);
        curve_curves_opacity = config->getDouble(CURVE_CURVES_OPACITY);
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *config) const override {
        config->setProperty(CURVE_PAINT_CONNECTION_LINE, curve_paint_connection_line);
        config->setProperty(CURVE_SMOOTHING, curve_smoothing);
        config->setProperty(CURVE_STROKE_HISTORY_SIZE, curve_stroke_history_size);
        config->setProperty(CURVE_LINE_WIDTH, curve_line_width);
        config->setProperty(CURVE_CURVES_OPACITY, curve_curves_opacity);
    }
};

#endif
