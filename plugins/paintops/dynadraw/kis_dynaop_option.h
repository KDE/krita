/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_DYNAOP_OPTION_H
#define KIS_DYNAOP_OPTION_H

#include <kis_paintop_option.h>

const QString DYNA_DIAMETER = "Dyna/diameter";
const QString DYNA_WIDTH = "Dyna/width";
const QString DYNA_MASS = "Dyna/mass";
const QString DYNA_DRAG = "Dyna/drag";
const QString DYNA_USE_FIXED_ANGLE = "Dyna/useFixedAngle";
const QString DYNA_ANGLE = "Dyna/angle";
const QString DYNA_WIDTH_RANGE = "Dyna/widthRange";
const QString DYNA_ACTION = "Dyna/action";
const QString DYNA_USE_TWO_CIRCLES = "Dyna/useTwoCirles";
const QString DYNA_ENABLE_LINE = "Dyna/enableLine";
const QString DYNA_LINE_COUNT = "Dyna/lineCount";
const QString DYNA_LINE_SPACING = "Dyna/lineSpacing";

class KisDynaOpOptionsWidget;
class KisPaintopLodLimitations;

class KisDynaOpOption : public KisPaintOpOption
{
    Q_OBJECT

public:
    KisDynaOpOption();
    ~KisDynaOpOption();

    qreal initWidth() const;
    qreal mass() const;
    qreal drag() const;
    bool useFixedAngle() const;
    qreal widthRange() const;

    int action() const;
    bool enableLine() const;
    bool useTwoCircles() const;

    int lineCount() const;
    qreal lineSpacing() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    void readOptionSetting(const KisPropertiesConfigurationSP setting);
    void lodLimitations(KisPaintopLodLimitations *l) const;

private:
    KisDynaOpOptionsWidget * m_options;
};

struct DynaOption {

    int dyna_action;

    qreal dyna_width;
    qreal dyna_mass;
    qreal dyna_drag;
    qreal dyna_angle;
    qreal dyna_width_range;
    int dyna_diameter;
    int dyna_line_count;
    qreal dyna_line_spacing;
    bool dyna_enable_line;
    bool dyna_use_two_circles;
    bool dyna_use_fixed_angle;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        setting->setProperty(DYNA_WIDTH, dyna_width);
        setting->setProperty(DYNA_MASS, dyna_mass);
        setting->setProperty(DYNA_DRAG, dyna_drag);
        setting->setProperty(DYNA_USE_FIXED_ANGLE, dyna_use_fixed_angle);
        setting->setProperty(DYNA_ANGLE, dyna_angle);
        setting->setProperty(DYNA_WIDTH_RANGE, dyna_width_range);
        setting->setProperty(DYNA_ACTION, dyna_action);
        setting->setProperty(DYNA_DIAMETER, dyna_diameter);
        setting->setProperty(DYNA_ENABLE_LINE, dyna_enable_line);
        setting->setProperty(DYNA_USE_TWO_CIRCLES, dyna_use_two_circles);
        setting->setProperty(DYNA_LINE_COUNT, dyna_line_count);
        setting->setProperty(DYNA_LINE_SPACING, dyna_line_spacing);

    }

    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        dyna_action = setting->getInt(DYNA_ACTION);
        dyna_width = setting->getDouble(DYNA_WIDTH);
        dyna_mass = setting->getDouble(DYNA_MASS);
        dyna_drag = setting->getDouble(DYNA_DRAG);
        dyna_angle = setting->getDouble(DYNA_ANGLE);
        dyna_width_range = setting->getDouble(DYNA_WIDTH_RANGE);
        dyna_diameter = setting->getInt(DYNA_DIAMETER);
        dyna_line_count = setting->getInt(DYNA_LINE_COUNT);
        dyna_line_spacing = setting->getDouble(DYNA_LINE_SPACING);
        dyna_enable_line = setting->getBool(DYNA_ENABLE_LINE);
        dyna_use_two_circles = setting->getBool(DYNA_USE_TWO_CIRCLES);
        dyna_use_fixed_angle = setting->getBool(DYNA_USE_FIXED_ANGLE);
    }
};


#endif
