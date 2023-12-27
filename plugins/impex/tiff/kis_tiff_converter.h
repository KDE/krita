/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TIFF_OPTIONS_H
#define KIS_TIFF_OPTIONS_H

#include <kis_types.h>

struct KisTIFFOptions {
    quint16 compressionType = 0;
    quint16 predictor = 1;
    // Disable alpha by default because the test suite
    // doesn't run the dialog's color space checks.
    bool alpha = false;
    bool saveAsPhotoshop = false;
    quint16 psdCompressionType = 0;
    bool flatten = true;
    quint16 jpegQuality = 80;
    quint16 deflateCompress = 6;
    quint16 pixarLogCompress = 6;
    bool saveProfile = true;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP cfg);
};

#endif // KIS_TIFF_OPTIONS_H
