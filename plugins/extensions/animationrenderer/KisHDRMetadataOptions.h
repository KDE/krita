/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
