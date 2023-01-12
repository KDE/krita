/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHatchingPreferencesData.h"

#include "kis_properties_configuration.h"


const QString HATCHING_ANTIALIAS = "Hatching/bool_antialias";
const QString HATCHING_OPAQUE_BACKGROUND = "Hatching/bool_opaquebackground";
const QString HATCHING_SUBPIXEL_PRECISION = "Hatching/bool_subpixelprecision";


bool KisHatchingPreferencesData::read(const KisPropertiesConfiguration *setting)
{
    useAntialias = setting->getBool(HATCHING_ANTIALIAS, false);
    useOpaqueBackground = setting->getBool(HATCHING_OPAQUE_BACKGROUND, false);
    useSubpixelPrecision = setting->getBool(HATCHING_SUBPIXEL_PRECISION, false);

    return true;
}

void KisHatchingPreferencesData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(HATCHING_ANTIALIAS, useAntialias);
    setting->setProperty(HATCHING_OPAQUE_BACKGROUND, useOpaqueBackground);
    setting->setProperty(HATCHING_SUBPIXEL_PRECISION, useSubpixelPrecision);
}
