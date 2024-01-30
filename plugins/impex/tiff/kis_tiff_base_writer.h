/*
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2024 Lucas Chollet <lucas.chollet@serenityos.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TIFF_BASE_WRITER_H
#define KIS_TIFF_BASE_WRITER_H

#include "tiffio.h"

#include <array>

#include <kis_types.h>

class KisTIFFOptions;
class KoColorSpace;

class KisTIFFBaseWriter
{
protected:
    KisTIFFBaseWriter(TIFF*image, KisTIFFOptions* options);
    ~KisTIFFBaseWriter() = default;

    static bool isBitDepthFloat(const KoID depth);
    static bool writeColorSpaceInformation(TIFF *image,
                                           const KoColorSpace *cs,
                                           uint16_t &color_type,
                                           uint16_t &sample_format,
                                           const KoColorSpace *&destColorSpace);

    inline TIFF *image()
    {
        return m_image;
    }

    bool copyDataToStrips(KisHLineConstIteratorSP it,
                          tdata_t buff,
                          uint32_t depth,
                          uint16_t sample_format,
                          uint8_t nbcolorssamples,
                          const std::array<quint8, 5> &poses);

    TIFF *m_image;
    KisTIFFOptions *m_options;
};

#endif
