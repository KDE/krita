/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
