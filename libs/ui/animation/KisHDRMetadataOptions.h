/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISHDRMETADATAOPTIONS_H
#define KISHDRMETADATAOPTIONS_H

#include <QString>
#include "kis_types.h"
#include <kis_hdr_metadata.h>

struct KisHDRMetadataOptions
{
    KisHDRMetadataOptions();

    QString predefinedMasterDisplayId;

    KisColorVolumeInformation cvi;

    KisContentLightLevelInformation clli;

    double diffuseWhite = 80;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP config);

    QString generateFFMpegOptions() const;
};

#endif // KISHDRMETADATAOPTIONS_H
