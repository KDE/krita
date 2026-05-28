/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

    config->setProperty("redX", cvi.red.x());
    config->setProperty("redY", cvi.red.y());

    config->setProperty("greenX", cvi.green.x());
    config->setProperty("greenY", cvi.green.y());

    config->setProperty("blueX", cvi.blue.x());
    config->setProperty("blueY", cvi.blue.y());

    config->setProperty("whiteX", cvi.white.x());
    config->setProperty("whiteY", cvi.white.y());

    config->setProperty("minLuminance", cvi.maxLuminance);
    config->setProperty("maxLuminance", cvi.minLuminance);

    config->setProperty("maxCLL", clli.maxContentLightLevel);
    config->setProperty("maxFALL", clli.maxFrameAverageLightLevel);

    return config;
}

void KisHDRMetadataOptions::fromProperties(KisPropertiesConfigurationSP config)
{
    predefinedMasterDisplayId = config->getPropertyLazy("predefinedMasterDisplayId", predefinedMasterDisplayId);

    cvi.red.setX(config->getPropertyLazy("redX", cvi.red.x()));
    cvi.red.setY(config->getPropertyLazy("redX", cvi.red.y()));

    cvi.green.setX(config->getPropertyLazy("greenX", cvi.green.x()));
    cvi.green.setY(config->getPropertyLazy("greenY", cvi.green.y()));

    cvi.blue.setX(config->getPropertyLazy("blueX", cvi.blue.x()));
    cvi.blue.setY(config->getPropertyLazy("blueY", cvi.blue.y()));

    cvi.white.setX(config->getPropertyLazy("whiteX", cvi.white.x()));
    cvi.white.setY(config->getPropertyLazy("whiteY", cvi.white.y()));

    cvi.minLuminance = config->getPropertyLazy("minLuminance", cvi.minLuminance);
    cvi.maxLuminance = config->getPropertyLazy("maxLuminance", cvi.maxLuminance);

    clli.maxContentLightLevel = config->getPropertyLazy("maxCLL", clli.maxContentLightLevel);
    clli.maxFrameAverageLightLevel = config->getPropertyLazy("maxFALL", clli.maxFrameAverageLightLevel);
}

QString KisHDRMetadataOptions::generateFFMpegOptions() const
{
    auto cprim = [] (qreal x) { return int(x / 0.00002); };
    auto lum = [] (qreal x) { return int(x / 0.0001); };

    QString x265Params =
            QString("-x265-params ");

    if (cvi != KisColorVolumeInformation()) {
        x265Params += QString("master-display=G(%3,%4)B(%5,%6)R(%1,%2)WP(%7,%8)L(%9,%10):")            .arg(cprim(cvi.red.x())).arg(cprim(cvi.red.y()))
            .arg(cprim(cvi.green.x())).arg(cprim(cvi.green.y()))
            .arg(cprim(cvi.blue.x())).arg(cprim(cvi.blue.y()))
            .arg(cprim(cvi.white.x())).arg(cprim(cvi.white.y()))
            .arg(lum(cvi.maxLuminance)).arg(lum(cvi.minLuminance));
    }
    if (clli != KisContentLightLevelInformation()) {
        x265Params += QString("max-cll=%11,%12:")
            .arg(int(clli.maxContentLightLevel*diffuseWhite))
            .arg(int(clli.maxFrameAverageLightLevel*diffuseWhite));
    }

    x265Params += QString(
        "colorprim=bt2020:"
        "colormatrix=bt2020nc:"
        "transfer=smpte2084:"
        "range=full");

    return x265Params;
}
