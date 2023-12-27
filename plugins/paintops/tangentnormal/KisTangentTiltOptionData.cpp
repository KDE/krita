/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTangentTiltOptionData.h"

#include "kis_properties_configuration.h"


const QString TANGENT_RED = "Tangent/swizzleRed";
const QString TANGENT_GREEN = "Tangent/swizzleGreen";
const QString TANGENT_BLUE = "Tangent/swizzleBlue";
const QString TANGENT_TYPE = "Tangent/directionType";
const QString TANGENT_EV_SEN = "Tangent/elevationSensitivity";
const QString TANGENT_MIX_VAL = "Tangent/mixValue";


bool KisTangentTiltOptionData::read(const KisPropertiesConfiguration *setting)
{
    redChannel = setting->getInt(TANGENT_RED, 0);
    greenChannel = setting->getInt(TANGENT_GREEN, 2);
    blueChannel = setting->getInt(TANGENT_BLUE, 4);

    directionType = (TangentTiltDirectionType)setting->getInt(TANGENT_TYPE, 0);

    elevationSensitivity = setting->getDouble(TANGENT_EV_SEN, 100.0);
    mixValue = setting->getDouble(TANGENT_MIX_VAL, 50.0);

    return true;
}

void KisTangentTiltOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(TANGENT_RED, redChannel);
    setting->setProperty(TANGENT_GREEN, greenChannel);
    setting->setProperty(TANGENT_BLUE, blueChannel);
    setting->setProperty(TANGENT_TYPE, directionType);
    setting->setProperty(TANGENT_EV_SEN, elevationSensitivity);
    setting->setProperty(TANGENT_MIX_VAL, mixValue);
}
