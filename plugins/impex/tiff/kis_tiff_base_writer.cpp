/*
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2024 Lucas Chollet <lucas.chollet@serenityos.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColorModelStandardIdsUtils.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoID.h>
#include <kis_iterator_ng.h>

#include "kis_tiff_base_writer.h"
#include "kis_tiff_converter.h"

KisTIFFBaseWriter::KisTIFFBaseWriter(TIFF *image, KisTIFFOptions *options)
    : m_image(image)
    , m_options(options)
{
}

bool KisTIFFBaseWriter::isBitDepthFloat(const KoID depth)
{
    return depth == Float16BitsColorDepthID || depth == Float32BitsColorDepthID || depth == Float64BitsColorDepthID;
}

bool KisTIFFBaseWriter::writeColorSpaceInformation(TIFF *image,
                                                   const KoColorSpace *cs,
                                                   uint16_t &color_type,
                                                   uint16_t &sample_format,
                                                   const KoColorSpace *&destColorSpace)
{
    const KoID id = cs->colorModelId();
    const KoID depth = cs->colorDepthId();
    // destColorSpace should be reassigned to a proper color space to convert to
    // if the return value of this function is false
    destColorSpace = nullptr;

    // sample_format and color_type should be assigned to the destination color
    // space, not /always/ the one we get here

    if (id == RGBAColorModelID) {
        color_type = PHOTOMETRIC_RGB;
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id == CMYKAColorModelID) {
        color_type = PHOTOMETRIC_SEPARATED;
        TIFFSetField(image, TIFFTAG_INKSET, INKSET_CMYK);

        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id == LABAColorModelID) {
        color_type = PHOTOMETRIC_ICCLAB;

        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id == GrayAColorModelID) {
        color_type = PHOTOMETRIC_MINISBLACK;
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;
    } else if (id == YCbCrAColorModelID) {
        color_type = PHOTOMETRIC_YCBCR;
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;
    } else {
        color_type = PHOTOMETRIC_RGB;
        destColorSpace =
            KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                         depth.id(),
                                                         KoColorSpaceRegistry::instance()->p709SRGBProfile());
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return false;
    }
}

bool KisTIFFBaseWriter::copyDataToStrips(KisHLineConstIteratorSP it,
                                         tdata_t buff,
                                         uint32_t depth,
                                         uint16_t sample_format,
                                         uint8_t nbcolorssamples,
                                         const std::array<quint8, 5> &poses)
{
    if (depth == 32) {
        Q_ASSERT(sample_format == SAMPLEFORMAT_IEEEFP);
        float *dst = reinterpret_cast<float *>(buff);
        do {
            const float *d = reinterpret_cast<const float *>(it->oldRawData());
            for (uint8_t i = 0; i < nbcolorssamples; i++) {
                *(dst++) = d[poses.at(i)];
            }
            if (m_options->alpha)
                *(dst++) = d[poses.at(nbcolorssamples)];
        } while (it->nextPixel());
        return true;
    } else if (depth == 16) {
        if (sample_format == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
            half *dst = reinterpret_cast<half *>(buff);
            do {
                const half *d = reinterpret_cast<const half *>(it->oldRawData());
                for (uint8_t i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses.at(i)];
                }
                if (m_options->alpha)
                    *(dst++) = d[poses.at(nbcolorssamples)];

            } while (it->nextPixel());
            return true;
#endif
        } else {
            quint16 *dst = reinterpret_cast<quint16 *>(buff);
            do {
                const quint16 *d = reinterpret_cast<const quint16 *>(it->oldRawData());
                for (uint8_t i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses.at(i)];
                }
                if (m_options->alpha)
                    *(dst++) = d[poses.at(nbcolorssamples)];

            } while (it->nextPixel());
            return true;
        }
    } else if (depth == 8) {
        quint8 *dst = reinterpret_cast<quint8 *>(buff);
        do {
            const quint8 *d = it->oldRawData();
            for (uint8_t i = 0; i < nbcolorssamples; i++) {
                *(dst++) = d[poses.at(i)];
            }
            if (m_options->alpha)
                *(dst++) = d[poses.at(nbcolorssamples)];

        } while (it->nextPixel());
        return true;
    }
    return false;
}
