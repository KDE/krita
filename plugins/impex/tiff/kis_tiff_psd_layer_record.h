/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_PSD_LAYER_RECORD_H
#define _KIS_TIFF_PSD_LAYER_RECORD_H

#include <cstdint>
#include <memory>
#include <psd.h>
#include <psd_layer_section.h>

class KisTiffPsdLayerRecord
{
public:
    KisTiffPsdLayerRecord(bool isBigEndian,
                          uint32_t width,
                          uint32_t height,
                          uint16_t channelDepth,
                          uint16_t nChannels,
                          uint16_t photometricInterpretation,
                          bool hasTransparency = false);

    bool read(QIODevice &io);

    bool write(QIODevice &io, KisNodeSP rootLayer, psd_compression_type compressionType);

    std::shared_ptr<PSDLayerMaskSection> record() const;

    psd_color_mode colorMode() const
    {
        return m_colorMode;
    }

    uint16_t channelDepth() const
    {
        return m_channelDepth;
    }

    bool valid() const;

private:
    psd_byte_order m_byteOrder;
    uint32_t m_width;
    uint32_t m_height;
    uint16_t m_channelDepth;
    uint16_t m_nChannels;
    psd_color_mode m_colorMode;
    std::shared_ptr<PSDLayerMaskSection> m_record;
    bool m_hasTransparency;
    bool m_valid;

private:
    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    bool readImpl(QIODevice &device);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    bool writeImpl(QIODevice &device, KisNodeSP rootLayer, psd_compression_type compressionType);
};

#endif // _KIS_TIFF_PSD_LAYER_RECORD_H
