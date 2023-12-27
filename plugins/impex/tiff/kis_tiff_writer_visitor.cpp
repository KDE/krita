/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QBuffer>

#include <memory>

#include <tiff.h>

#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoID.h>
#include <kis_assert.h>
#include <kis_meta_data_backend_registry.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "kis_tiff_converter.h"
#include "kis_tiff_writer_visitor.h"

namespace
{
bool isBitDepthFloat(const KoID depth)
{
    return depth == Float16BitsColorDepthID || depth == Float32BitsColorDepthID
        || depth == Float64BitsColorDepthID;
}

bool writeColorSpaceInformation(TIFF *image,
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
    } else if (id == LABAColorModelID) {
        color_type = PHOTOMETRIC_CIELAB;
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
        destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            depth.id(),
            KoColorSpaceRegistry::instance()->p709SRGBProfile());
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return false;
    }
}
} // namespace

KisTIFFWriterVisitor::KisTIFFWriterVisitor(TIFF*image, KisTIFFOptions* options)
    : m_image(image)
    , m_options(options)
{
}

KisTIFFWriterVisitor::~KisTIFFWriterVisitor() = default;

bool KisTIFFWriterVisitor::copyDataToStrips(KisHLineConstIteratorSP it,
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
    }
    else if (depth == 16 ) {
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
        }
        else {
            quint16 *dst = reinterpret_cast<quint16 *>(buff);
            do {
                const quint16 *d =
                    reinterpret_cast<const quint16 *>(it->oldRawData());
                for (uint8_t i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses.at(i)];
                }
                if (m_options->alpha)
                    *(dst++) = d[poses.at(nbcolorssamples)];

            } while (it->nextPixel());
            return true;
        }
    }
    else if (depth == 8) {
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

bool KisTIFFWriterVisitor::saveLayerProjection(KisLayer *layer)
{
    dbgFile << "visiting on layer" << layer->name() << "";
    KisPaintDeviceSP pd = layer->projection();

    uint16_t color_type = 0;
    uint16_t sample_format = SAMPLEFORMAT_UINT;
    const KoColorSpace *destColorSpace = nullptr;
    // Check colorspace
    if (!writeColorSpaceInformation(image(), pd->colorSpace(), color_type, sample_format, destColorSpace)) { // unsupported colorspace
        if (!destColorSpace) {
            return false;
        }
        pd.attach(new KisPaintDevice(*pd));
        pd->convertTo(destColorSpace);
    }

    {
        // WORKAROUND: block any attempts to use YCbCr with alpha channels.
        // This should not happen because alpha is disabled by default
        // and the checkbox is blocked for YCbCr and CMYK.
        KIS_SAFE_ASSERT_RECOVER(color_type != PHOTOMETRIC_YCBCR
                                || !m_options->alpha)
        {
            warnFile << "TIFF does not support exporting alpha channels with "
                        "YCbCr. Skipping...";
            m_options->alpha = false;
        }
    }

    // Save depth
    uint32_t depth = 8 * pd->pixelSize() / pd->channelCount();
    TIFFSetField(image(), TIFFTAG_BITSPERSAMPLE, depth);

    {
        // WORKAROUND: block any attempts to use JPEG with >= 8 bits

        if (m_options->compressionType == COMPRESSION_JPEG && depth != 8) {
            warnFile << "Attempt to export JPEG with multi-byte depth, "
                        "disabling compression";
            m_options->compressionType = COMPRESSION_NONE;
        }
    }

    // Save number of samples
    if (m_options->alpha) {
        TIFFSetField(image(), TIFFTAG_SAMPLESPERPIXEL, pd->channelCount());
        const std::array<uint16_t, 1> sampleinfo = {EXTRASAMPLE_UNASSALPHA};
        TIFFSetField(image(), TIFFTAG_EXTRASAMPLES, 1, sampleinfo.data());
    } else {
        TIFFSetField(image(), TIFFTAG_SAMPLESPERPIXEL, pd->channelCount() - 1);
        TIFFSetField(image(), TIFFTAG_EXTRASAMPLES, 0);
    }

    // Save colorspace information
    TIFFSetField(image(), TIFFTAG_PHOTOMETRIC, color_type);
    TIFFSetField(image(), TIFFTAG_SAMPLEFORMAT, sample_format);
    TIFFSetField(image(), TIFFTAG_IMAGEWIDTH, layer->image()->width());
    TIFFSetField(image(), TIFFTAG_IMAGELENGTH, layer->image()->height());

    // Set the compression options
    TIFFSetField(image(), TIFFTAG_COMPRESSION, m_options->compressionType);
    if (m_options->compressionType == COMPRESSION_JPEG) {
        TIFFSetField(image(), TIFFTAG_JPEGQUALITY, m_options->jpegQuality);
    } else if (m_options->compressionType == COMPRESSION_DEFLATE) {
        TIFFSetField(image(), TIFFTAG_ZIPQUALITY, m_options->deflateCompress);
    } else if (m_options->compressionType == COMPRESSION_PIXARLOG) {
        TIFFSetField(image(),
                     TIFFTAG_PIXARLOGQUALITY,
                     m_options->pixarLogCompress);
    }

    // Set the predictor
    if (m_options->compressionType == COMPRESSION_LZW
        || m_options->compressionType == COMPRESSION_ADOBE_DEFLATE
        || m_options->compressionType == COMPRESSION_DEFLATE)
        TIFFSetField(image(), TIFFTAG_PREDICTOR, m_options->predictor);

    // Use contiguous configuration
    TIFFSetField(image(), TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Do not set the rowsperstrip, as it's incompatible with JPEG

    // But do set YCbCr 4:4:4 if applicable
    if (color_type == PHOTOMETRIC_YCBCR) {
        TIFFSetField(image(), TIFFTAG_YCBCRSUBSAMPLING, 1, 1);
        TIFFSetField(image(), TIFFTAG_YCBCRPOSITIONING, YCBCRPOSITION_CENTERED);
        if (m_options->compressionType == COMPRESSION_JPEG) {
            TIFFSetField(image(), TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RAW);
        }
    }

    // Save profile
    if (m_options->saveProfile) {
        const KoColorProfile* profile = pd->colorSpace()->profile();
        if (profile && profile->type() == "icc" && !profile->rawData().isEmpty()) {
            QByteArray ba = profile->rawData();
            TIFFSetField(image(), TIFFTAG_ICCPROFILE, ba.size(), ba.constData());
        }
    }

    {
        // IPTC
        KisMetaData::IOBackend *io =
            KisMetadataBackendRegistry::instance()->value("iptc");
        QBuffer buf;
        io->saveTo(layer->metaData(), &buf, KisMetaData::IOBackend::NoHeader);

        if (buf.size()
            && !TIFFSetField(image(),
                             TIFFTAG_RICHTIFFIPTC,
                             static_cast<uint32_t>(buf.size()),
                             buf.data().data())) {
            dbgFile << "Failed to write the IPTC metadata to the TIFF field";
        }
    }

    {
        // XMP
        KisMetaData::IOBackend *io =
            KisMetadataBackendRegistry::instance()->value("xmp");
        QBuffer buf;
        io->saveTo(layer->metaData(), &buf, KisMetaData::IOBackend::NoHeader);

        if (buf.size()
            && !TIFFSetField(image(),
                             TIFFTAG_XMLPACKET,
                             static_cast<uint32_t>(buf.size()),
                             buf.data().data())) {
            dbgFile << "Failed to write the XMP metadata to the TIFF field";
        }
    }

    tsize_t stripsize = TIFFStripSize(image());
    std::unique_ptr<std::remove_pointer_t<tdata_t>, decltype(&_TIFFfree)> buff(
        _TIFFmalloc(stripsize),
        &_TIFFfree);
    KIS_ASSERT_RECOVER_RETURN_VALUE(
        buff && "Unable to allocate buffer for TIFF!",
        false);
    qint32 height = layer->image()->height();
    qint32 width = layer->image()->width();
    bool r = true;
    for (int y = 0; y < height; y++) {
        KisHLineConstIteratorSP it = pd->createHLineConstIteratorNG(0, y, width);
        switch (color_type) {
        case PHOTOMETRIC_MINISBLACK: {
            const std::array<quint8, 5> poses = {0, 1};
            r = copyDataToStrips(it,
                                 buff.get(),
                                 depth,
                                 sample_format,
                                 1,
                                 poses);
            }
            break;
        case PHOTOMETRIC_RGB: {
            const auto poses = [&]() -> std::array<quint8, 5> {
                if (sample_format == SAMPLEFORMAT_IEEEFP) {
                    return {0, 1, 2, 3};
                } else {
                    return {2, 1, 0, 3};
                }
            }();
            r = copyDataToStrips(it,
                                 buff.get(),
                                 depth,
                                 sample_format,
                                 3,
                                 poses);
            }
            break;
        case PHOTOMETRIC_SEPARATED: {
            const std::array<quint8, 5> poses = {0, 1, 2, 3, 4};
            r = copyDataToStrips(it,
                                 buff.get(),
                                 depth,
                                 sample_format,
                                 4,
                                 poses);
            }
            break;
            case PHOTOMETRIC_ICCLAB:
            case PHOTOMETRIC_YCBCR: {
                const std::array<quint8, 5> poses = {0, 1, 2, 3};
                r = copyDataToStrips(it,
                                     buff.get(),
                                     depth,
                                     sample_format,
                                     3,
                                     poses);
            } break;
        }
        if (!r) return false;
        TIFFWriteScanline(image(),
                          buff.get(),
                          static_cast<uint32_t>(y),
                          (tsample_t)-1);
    }
    buff.reset();

    return TIFFWriteDirectory(image());
}
