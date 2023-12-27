/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRoundMarkerOpOptionData.h"

#include "kis_properties_configuration.h"


const QString ROUNDMARKER_DIAMETER = "diameter";
const QString ROUNDMARKER_SPACING = "spacing";
const QString ROUNDMARKER_USE_AUTO_SPACING = "useAutoSpacing";
const QString ROUNDMARKER_AUTO_SPACING_COEFF = "autoSpacingCoeff";


bool KisRoundMarkerOpOptionData::read(const KisPropertiesConfiguration *setting)
{
    diameter = setting->getDouble(ROUNDMARKER_DIAMETER, 30.0);
    spacing = setting->getDouble(ROUNDMARKER_SPACING, 0.02);
    useAutoSpacing = setting->getBool(ROUNDMARKER_USE_AUTO_SPACING, false);
    autoSpacingCoeff = setting->getDouble(ROUNDMARKER_AUTO_SPACING_COEFF, 1.0);

    return true;
}

void KisRoundMarkerOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(ROUNDMARKER_DIAMETER, diameter);
    setting->setProperty(ROUNDMARKER_SPACING, spacing);
    setting->setProperty(ROUNDMARKER_USE_AUTO_SPACING, useAutoSpacing);
    setting->setProperty(ROUNDMARKER_AUTO_SPACING_COEFF, autoSpacingCoeff);
}
