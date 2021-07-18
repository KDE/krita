/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_psd_layer_record.h"

#include <memory>

#include <asl/kis_asl_reader_utils.h>
#include <kis_debug.h>
#include <psd.h>
#include <tiff.h>

KisTiffPsdLayerRecord::KisTiffPsdLayerRecord(bool isBigEndian,
                                             uint32_t width,
                                             uint32_t height,
                                             uint16_t channelDepth,
                                             uint16_t nChannels,
                                             uint16_t photometricInterpretation)
    : m_byteOrder(isBigEndian ? psd_byte_order::psdBigEndian : psd_byte_order::psdLittleEndian)
    , m_width(width)
    , m_height(height)
    , m_channelDepth(channelDepth)
    , m_nChannels(nChannels)
    , m_valid(false)
{
    if (photometricInterpretation == PHOTOMETRIC_MINISWHITE || photometricInterpretation == PHOTOMETRIC_MINISBLACK) {
        m_colorMode = psd_color_mode::Grayscale;
    } else if (photometricInterpretation == PHOTOMETRIC_RGB) {
        m_colorMode = psd_color_mode::RGB;
    } else if (photometricInterpretation == PHOTOMETRIC_SEPARATED) {
        dbgFile << "PSD warning: assuming CMYK for color separations";
        m_colorMode = psd_color_mode::CMYK;
    } else if (photometricInterpretation == PHOTOMETRIC_CIELAB || photometricInterpretation == PHOTOMETRIC_ICCLAB) {
        m_colorMode = psd_color_mode::Lab;
    } else if (photometricInterpretation == PHOTOMETRIC_PALETTE) {
        m_colorMode = psd_color_mode::Indexed;
    } else {
        dbgFile << "TIFF PSD error: " << photometricInterpretation << "does not map to a Photoshop supported mode";
        m_colorMode = psd_color_mode::COLORMODE_UNKNOWN;
    }
}

bool KisTiffPsdLayerRecord::read(QIODevice &io)
{
    switch (m_byteOrder) {
    case psd_byte_order::psdLittleEndian:
        return readImpl<psd_byte_order::psdLittleEndian>(io);
    default:
        return readImpl(io);
    }
}

template<psd_byte_order byteOrder>
bool KisTiffPsdLayerRecord::readImpl(QIODevice &device)
{
    PSDHeader header;
    header.version = 1;
    header.byteOrder = byteOrder;
    header.width = m_width;
    header.height = m_height;
    header.channelDepth = m_channelDepth;
    header.nChannels = m_nChannels;
    header.colormode = m_colorMode;
    header.tiffStyleLayerBlock = true;
    m_record = std::make_shared<PSDLayerMaskSection>(header);

    QLatin1String signature("Adobe Photoshop Document Data Block");
    QByteArray b = device.read(signature.size() + 1);
    if (b.size() != signature.size() + 1 || QLatin1String(b) != signature) {
        m_record->error = QString("Invalid Photoshop data block: %1").arg(QLatin1String(b));
        return false;
    }

    if (!m_record->read(device)) {
        dbgFile << "failed reading PSD section: " << m_record->error;
        return false;
    }

    dbgFile << "Resource section: " << m_record->nLayers;

    m_valid = true;

    return true;
}

std::shared_ptr<PSDLayerMaskSection> KisTiffPsdLayerRecord::record() const
{
    return m_record;
}

bool KisTiffPsdLayerRecord::valid() const
{
    return m_valid;
}
