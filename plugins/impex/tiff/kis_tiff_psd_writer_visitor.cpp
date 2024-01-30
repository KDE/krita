/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
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
#include <KoConfig.h>
#include <KoID.h>
#include <KoUnit.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_painter.h>
#include <kis_tiff_psd_resource_record.h>
#include <psd_resource_block.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "kis_tiff_converter.h"
#include "kis_tiff_psd_layer_record.h"
#include "kis_tiff_psd_writer_visitor.h"

KisTiffPsdWriter::KisTiffPsdWriter(TIFF *image, KisTIFFOptions *options)
    : KisTIFFBaseWriter(image, options)
{
}

KisTiffPsdWriter::~KisTiffPsdWriter() = default;

KisImportExportErrorCode KisTiffPsdWriter::writeImage(KisGroupLayerSP layer)
{
    dbgFile << "Starting write of Photoshop layer data";

    /**
     * For Photoshop tiff files, first thing is to write the
     * projection of the image file.
     */

    if (layer->image()->width() > MAX_PSD_SIZE || layer->image()->height() > MAX_PSD_SIZE) {
        dbgFile << "This TIFF file is too big to be represented as a PSD blob!";
        return ImportExportCodes::Failure;
    }

    dbgFile << "Writing root layer projection";
    KisPaintDeviceSP pd = layer->projection();

    uint16_t color_type = 0;
    uint16_t sample_format = SAMPLEFORMAT_UINT;
    const KoColorSpace *destColorSpace = nullptr;
    // Check colorspace
    if (!writeColorSpaceInformation(image(), pd->colorSpace(), color_type, sample_format, destColorSpace)) { // unsupported colorspace
        if (!destColorSpace) {
            dbgFile << "Unsupported colorspace" << pd->colorSpace()->name();
            return ImportExportCodes::FormatColorSpaceUnsupported;
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
    } else if (m_options->compressionType == COMPRESSION_ADOBE_DEFLATE) {
        TIFFSetField(image(), TIFFTAG_ZIPQUALITY, m_options->deflateCompress);
    } else if (m_options->compressionType == COMPRESSION_PIXARLOG) {
        TIFFSetField(image(),
                     TIFFTAG_PIXARLOGQUALITY,
                     m_options->pixarLogCompress);
    }

    // Set the predictor
    if (m_options->compressionType == COMPRESSION_LZW
        || m_options->compressionType == COMPRESSION_ADOBE_DEFLATE)
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
        const KoColorProfile *profile = pd->colorSpace()->profile();
        if (profile && profile->type() == "icc" && !profile->rawData().isEmpty()) {
            QByteArray ba = profile->rawData();
            TIFFSetField(image(), TIFFTAG_ICCPROFILE, ba.size(), ba.constData());
        }
    }
    tsize_t stripsize = TIFFStripSize(image());
    std::unique_ptr<std::remove_pointer_t<tdata_t>, decltype(&_TIFFfree)> buff(
        _TIFFmalloc(stripsize),
        &_TIFFfree);
    KIS_ASSERT_RECOVER_RETURN_VALUE(
        buff && "Unable to allocate buffer for TIFF!",
        ImportExportCodes::InsufficientMemory);
    qint32 height = layer->image()->height();
    qint32 width = layer->image()->width();
    bool r = true;
    for (qint32 y = 0; y < height; y++) {
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
        } break;
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
        } break;
        case PHOTOMETRIC_SEPARATED: {
            const std::array<quint8, 5> poses = {0, 1, 2, 3, 4};
            r = copyDataToStrips(it,
                                 buff.get(),
                                 depth,
                                 sample_format,
                                 4,
                                 poses);
        } break;
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
        if (!r)
            return ImportExportCodes::InternalError;
        TIFFWriteScanline(image(),
                          buff.get(),
                          static_cast<quint32>(y),
                          (tsample_t)-1);
    }
    buff.reset();

    ///* BEGIN PHOTOSHOP SPECIFIC HANDLING CODE *///

    /**
     * Synthesize the PSD file into the TIFFTAG_IMAGESOURCEDATA field.
     */

    {
        const bool haveLayers = layer->childCount() > 1 || KisPainter::checkDeviceHasTransparency(layer->firstChild()->projection());

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);

        dbgFile << "m_image->rootLayer->childCount" << layer->childCount() << buf.pos();

        if (haveLayers) {
            KisTiffPsdLayerRecord layerSection(
                TIFFIsBigEndian(image()),
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                static_cast<uint16_t>(depth),
                static_cast<uint16_t>(pd->channelCount()),
                color_type,
                true);

            if (!layerSection.write(buf, layer, static_cast<psd_compression_type>(m_options->psdCompressionType))) {
                dbgFile << "failed to write layer section. Error:" << layerSection.record()->error << buf.pos();
                return ImportExportCodes::ErrorWhileWriting;
            }
        } else {
            // else write a zero length block
            dbgFile << "No layers, saving empty layers/mask block" << buf.pos();
            psdwrite(buf, (quint32)0);
        }

        buf.close();
        buf.open(QIODevice::ReadOnly);

        if (!TIFFSetField(image(), TIFFTAG_IMAGESOURCEDATA, static_cast<uint32_t>(buf.size()), buf.data().constData())) {
            dbgFile << "Failed to write the PSD image block to the TIFF field";
            return ImportExportCodes::ErrorWhileWriting;
        }
    }

    /**
     * Write all the annotations into the TIFFTAG_PHOTOSHOP field.
     */

    {
        // IMAGE RESOURCES SECTION
        KisTiffPsdResourceRecord resourceSection;

        for (vKisAnnotationSP_it it = layer->image()->beginAnnotations(); it != layer->image()->endAnnotations(); ++it) {
            KisAnnotationSP annotation = (*it);
            if (!annotation || annotation->type().isEmpty()) {
                dbgFile << "Warning: empty annotation";
                continue;
            }

            dbgFile << "Annotation:" << annotation->type() << annotation->description();

            if (annotation->type().startsWith(QString("PSD Resource Block:"))) { //
                PSDResourceBlock *resourceBlock =
                    dynamic_cast<PSDResourceBlock *>(annotation.data());
                if (resourceBlock) {
                    dbgFile << "Adding PSD Resource Block" << resourceBlock->identifier;
                    resourceSection.resources[(KisTiffPsdResourceRecord::PSDResourceID)resourceBlock->identifier] = resourceBlock;
                }
            }
        }

        // Add resolution block
        {
            auto *resInfo = new RESN_INFO_1005();
            resInfo->hRes = static_cast<int>(INCH_TO_POINT(layer->image()->xRes()));
            resInfo->vRes = static_cast<int>(INCH_TO_POINT(layer->image()->yRes()));
            auto *block = new PSDResourceBlock();
            block->identifier = KisTiffPsdResourceRecord::RESN_INFO;
            block->resource = resInfo;
            resourceSection.resources[KisTiffPsdResourceRecord::RESN_INFO] = block;
        }

        // Add icc block
        {
            auto *profileInfo = new ICC_PROFILE_1039();
            profileInfo->icc = layer->image()->profile()->rawData();
            auto *block = new PSDResourceBlock();
            block->identifier = KisTiffPsdResourceRecord::ICC_PROFILE;
            block->resource = profileInfo;
            resourceSection.resources[KisTiffPsdResourceRecord::ICC_PROFILE] = block;
        }

        dbgFile << "Resource section ready to write";

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);

        if (!resourceSection.write(buf)) {
            dbgFile << "Failed to write resource section. Error:" << resourceSection.error << buf.pos();
            return ImportExportCodes::ErrorWhileWriting;
        }

        buf.close();
        buf.open(QIODevice::WriteOnly);

        if (!TIFFSetField(image(), TIFFTAG_PHOTOSHOP, static_cast<uint32_t>(buf.size()), buf.data().data())) {
            dbgFile << "Failed to write the PSD resource block to the TIFF field";
            return ImportExportCodes::ErrorWhileWriting;
        }
    }

    /**
     * Freshly baked Photoshoppy TIFF file!
     */

    ///* END PHOTOSHOP SPECIFIC HANDLING CODE *///

    TIFFWriteDirectory(image());
    return ImportExportCodes::OK;
}
