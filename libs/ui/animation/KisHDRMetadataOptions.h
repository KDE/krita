/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISHDRMETADATAOPTIONS_H
#define KISHDRMETADATAOPTIONS_H

#include <QString>
#include "kis_types.h"

struct KisHDRMetadataOptions
{
    KisHDRMetadataOptions();

    QString predefinedMasterDisplayId;

    qreal redX = 0.708;
    qreal redY = 0.292;

    qreal greenX = 0.170;
    qreal greenY = 0.797;

    qreal blueX = 0.131;
    qreal blueY = 0.046;

    qreal whiteX = 0.3127;
    qreal whiteY = 0.3290;

    qreal minLuminance = 0.005;
    qreal maxLuminance = 1000;

    qreal maxCLL = 1000;
    qreal maxFALL = 400;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP config);

    QString generateFFMpegOptions() const;
};

#endif // KISHDRMETADATAOPTIONS_H
