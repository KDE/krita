/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_converter.h"

#include <tiffio.h>

#include <kis_properties_configuration.h>
#include <psd.h>

KisPropertiesConfigurationSP KisTIFFOptions::toProperties() const
{
    QHash<int, int> compToIndex;
    compToIndex[COMPRESSION_NONE] = 0;
    compToIndex[COMPRESSION_JPEG] = 1;
    compToIndex[COMPRESSION_ADOBE_DEFLATE] = 2;
    compToIndex[COMPRESSION_LZW] = 3;
    compToIndex[COMPRESSION_PIXARLOG] = 8;

    const QHash<quint16, int> psdCompToIndex = {
        {psd_compression_type::RLE, 0},
        {psd_compression_type::ZIP, 1},
    };

    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    cfg->setProperty("compressiontype", compToIndex.value(compressionType, 0));
    cfg->setProperty("predictor", predictor - 1);
    cfg->setProperty("alpha", alpha);
    cfg->setProperty("psdCompressionType",
                     psdCompToIndex.value(psdCompressionType, 0));
    cfg->setProperty("saveAsPhotoshop", saveAsPhotoshop);
    cfg->setProperty("flatten", flatten);
    cfg->setProperty("quality", jpegQuality);
    cfg->setProperty("deflate", deflateCompress);
    cfg->setProperty("pixarlog", pixarLogCompress);
    cfg->setProperty("saveProfile", saveProfile);

    return cfg;
}

void KisTIFFOptions::fromProperties(KisPropertiesConfigurationSP cfg)
{
    QHash<int, int> indexToComp;
    indexToComp[0] = COMPRESSION_NONE;
    indexToComp[1] = COMPRESSION_JPEG;
    indexToComp[2] = COMPRESSION_ADOBE_DEFLATE;
    indexToComp[3] = COMPRESSION_LZW;
    indexToComp[4] = COMPRESSION_PIXARLOG;

    // old value that might be still stored in a config (remove after Krita 5.0
    // :) )
    indexToComp[8] = COMPRESSION_PIXARLOG;

    const QHash<int, quint16> psdIndexToComp = {
        {0, psd_compression_type::RLE},
        {1, psd_compression_type::ZIP},
    };

    compressionType = static_cast<quint16>(
        indexToComp.value(cfg->getInt("compressiontype", 0), COMPRESSION_NONE));

    predictor = static_cast<quint16>(cfg->getInt("predictor", 0)) + 1;
    alpha = cfg->getBool("alpha", false);
    saveAsPhotoshop = cfg->getBool("saveAsPhotoshop", false);
    psdCompressionType =
        psdIndexToComp.value(cfg->getInt("psdCompressionType", 0),
                             psd_compression_type::RLE);
    flatten = cfg->getBool("flatten", true);
    jpegQuality = static_cast<quint16>(cfg->getInt("quality", 80));
    deflateCompress = static_cast<quint16>(cfg->getInt("deflate", 6));
    pixarLogCompress = static_cast<quint16>(cfg->getInt("pixarlog", 6));
    saveProfile = cfg->getBool("saveProfile", true);
}
