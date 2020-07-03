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

#include "KisHDRMetadataOptions.h"

#include "kis_properties_configuration.h"


KisHDRMetadataOptions::KisHDRMetadataOptions()
    : predefinedMasterDisplayId("p2100-pq")
{
}

KisPropertiesConfigurationSP KisHDRMetadataOptions::toProperties() const
{
    KisPropertiesConfigurationSP config = new KisPropertiesConfiguration();

    config->setProperty("predefinedMasterDisplayId", predefinedMasterDisplayId);

    config->setProperty("redX", redX);
    config->setProperty("redY", redY);

    config->setProperty("greenX", greenX);
    config->setProperty("greenY", greenY);

    config->setProperty("blueX", blueX);
    config->setProperty("blueY", blueY);

    config->setProperty("whiteX", whiteX);
    config->setProperty("whiteY", whiteY);

    config->setProperty("minLuminance", minLuminance);
    config->setProperty("maxLuminance", maxLuminance);

    config->setProperty("maxCLL", maxCLL);
    config->setProperty("maxFALL", maxFALL);

    return config;
}

void KisHDRMetadataOptions::fromProperties(KisPropertiesConfigurationSP config)
{
    predefinedMasterDisplayId = config->getPropertyLazy("predefinedMasterDisplayId", predefinedMasterDisplayId);

    redX = config->getPropertyLazy("redX", redX);
    redY = config->getPropertyLazy("redY", redY);

    greenX = config->getPropertyLazy("greenX", greenX);
    greenY = config->getPropertyLazy("greenY", greenY);

    blueX = config->getPropertyLazy("blueX", blueX);
    blueY = config->getPropertyLazy("blueY", blueY);

    whiteX = config->getPropertyLazy("whiteX", whiteX);
    whiteY = config->getPropertyLazy("whiteY", whiteY);

    minLuminance = config->getPropertyLazy("minLuminance", minLuminance);
    maxLuminance = config->getPropertyLazy("maxLuminance", maxLuminance);

    maxCLL = config->getPropertyLazy("maxCLL", maxCLL);
    maxFALL = config->getPropertyLazy("maxFALL", maxFALL);
}

QString KisHDRMetadataOptions::generateFFMpegOptions() const
{
    auto cprim = [] (qreal x) { return int(x / 0.00002); };
    auto lum = [] (qreal x) { return int(x / 0.0001); };

    const QString x265Params =
            QString("-x265-params "
                "master-display=R(%1,%2)G(%3,%4)B(%5,%6)WP(%7,%8)L(%9,%10):"
                "max-cll=%11,%12:"
                "colorprim=bt2020:"
                "colormatrix=bt2020c:"
                "transfer=smpte2084:"
                "range=full")
            .arg(cprim(redX)).arg(cprim(redY))
            .arg(cprim(greenX)).arg(cprim(greenY))
            .arg(cprim(blueX)).arg(cprim(blueY))
            .arg(cprim(whiteX)).arg(cprim(whiteY))
            .arg(lum(maxLuminance)).arg(lum(minLuminance))
            .arg(int(maxCLL)).arg(int(maxFALL));

    return x265Params;
}
