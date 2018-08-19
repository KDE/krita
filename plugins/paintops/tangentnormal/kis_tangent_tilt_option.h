/* This file is part of the KDE project
 *
 * Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TANGENT_TILT_OPTION_H
#define KIS_TANGENT_TILT_OPTION_H

#include <brushengine/kis_paint_information.h>
#include <kis_types.h>

#include <kis_paintop_option.h>

const QString TANGENT_RED = "Tangent/swizzleRed";
const QString TANGENT_GREEN = "Tangent/swizzleGreen";
const QString TANGENT_BLUE = "Tangent/swizzleBlue";
const QString TANGENT_TYPE = "Tangent/directionType";
const QString TANGENT_EV_SEN = "Tangent/elevationSensitivity";
const QString TANGENT_MIX_VAL = "Tangent/mixValue";
//const QString TANGENT_DIR_MIN = "Tangent/directionMinimum";
//const QString TANGENT_DIR_MAX = "Tangent/directionMaximum";

class KisPropertiesConfiguration;
class KisTangentTiltOptionWidget;

class KisTangentTiltOption: public KisPaintOpOption //not really//
{
public:
    KisTangentTiltOption();
    ~KisTangentTiltOption() override;
    /*These three give away which the index of the combobox for a given channel*/
    int redChannel() const;
    int greenChannel() const;
    int blueChannel() const;
    int directionType() const;
    double elevationSensitivity() const;
    double mixValue() const;
    qreal m_canvasAngle;
    bool m_canvasAxisXMirrored;
    bool m_canvasAxisYMirrored;
    /*This assigns the right axis to the component, based on index and maximum value*/
    void swizzleAssign(qreal const horizontal, qreal const vertical, qreal const depth, qreal *component, int index, qreal maxvalue);

    //takes the RGB values and will deform them depending on tilt.
    void apply(const KisPaintInformation& info, qreal *r, qreal *g, qreal *b);

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
private:
    KisTangentTiltOptionWidget * m_options;
};

#endif // KIS_TANGENT_TILT_OPTION_H
