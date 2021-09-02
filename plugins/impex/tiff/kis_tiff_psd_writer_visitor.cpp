/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_psd_writer_visitor.h"

#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoConfig.h>
#include <KoID.h>
#include <KoUnit.h>
#include <QBuffer>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_painter.h>
#include <kis_tiff_psd_resource_record.h>
#include <psd_resource_block.h>
#include <tiff.h>
#include <tiffio.h>

#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include "kis_tiff_psd_layer_record.h"

namespace
{
inline bool isBitDepthFloat(QString depth)
{
    return depth.contains("F");
}

inline bool writeColorSpaceInformation(TIFF *image, const KoColorSpace *cs, uint16_t &color_type, uint16_t &sample_format, const KoColorSpace *&destColorSpace)
{
    dbgKrita << cs->id();
    QString id = cs->id();
    QString depth = cs->colorDepthId().id();
    // destColorSpace should be reassigned to a proper color space to convert to
    // if the return value of this function is false
    destColorSpace = 0;

    // sample_format and color_type should be assigned to the destination color space,
    // not /always/ the one we get here

    if (id.contains("RGBA")) {
        color_type = PHOTOMETRIC_RGB;
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id.contains("CMYK")) {
        color_type = PHOTOMETRIC_SEPARATED;
        TIFFSetField(image, TIFFTAG_INKSET, INKSET_CMYK);

        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id.contains("LABA")) {
        color_type = PHOTOMETRIC_ICCLAB;

        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else if (id.contains("GRAYA")) {
        color_type = PHOTOMETRIC_MINISBLACK;
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return true;

    } else {
        color_type = PHOTOMETRIC_RGB;
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
        destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), depth, profile);
        if (isBitDepthFloat(depth)) {
            sample_format = SAMPLEFORMAT_IEEEFP;
        }
        return false;
    }
}
}

KisTiffPsdWriter::KisTiffPsdWriter(TIFF *image, KisTIFFOptions *options)
    : m_image(image)
    , m_options(options)
{
}

KisTiffPsdWriter::~KisTiffPsdWriter()
{
}

bool KisTiffPsdWriter::copyDataToStrips(KisHLineConstIteratorSP it,
                                        tdata_t buff,
                                        uint32_t depth,
                                        uint16_t sample_format,
                                        uint8_t nbcolorssamples,
                                        quint8 *poses)
{
    if (depth == 32) {
        Q_ASSERT(sample_format == SAMPLEFORMAT_IEEEFP);
        float *dst = reinterpret_cast<float *>(buff);
        do {
            const float *d = reinterpret_cast<const float *>(it->oldRawData());
            int i;
            for (i = 0; i < nbcolorssamples; i++) {
                *(dst++) = d[poses[i]];
            }
            if (m_options->alpha)
                *(dst++) = d[poses[i]];
        } while (it->nextPixel());
        return true;
    } else if (depth == 16) {
        if (sample_format == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
            half *dst = reinterpret_cast<half *>(buff);
            do {
                const half *d = reinterpret_cast<const half *>(it->oldRawData());
                int i;
                for (i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses[i]];
                }
                if (m_options->alpha)
                    *(dst++) = d[poses[i]];

            } while (it->nextPixel());
            return true;
#endif
        } else {
            quint16 *dst = reinterpret_cast<quint16 *>(buff);
            do {
                const quint16 *d = reinterpret_cast<const quint16 *>(it->oldRawData());
                int i;
                for (i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses[i]];
                }
                if (m_options->alpha)
                    *(dst++) = d[poses[i]];

            } while (it->nextPixel());
            return true;
        }
    } else if (depth == 8) {
        quint8 *dst = reinterpret_cast<quint8 *>(buff);
        do {
            const quint8 *d = it->oldRawData();
            int i;
            for (i = 0; i < nbcolorssamples; i++) {
                *(dst++) = d[poses[i]];
            }
            if (m_options->alpha)
                *(dst++) = d[poses[i]];

        } while (it->nextPixel());
        return true;
    }
    return false;
}

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

    uint16_t color_type;
    uint16_t sample_format = SAMPLEFORMAT_UINT;
    const KoColorSpace *destColorSpace;
    // Check colorspace
    if (!writeColorSpaceInformation(image(), pd->colorSpace(), color_type, sample_format, destColorSpace)) { // unsupported colorspace
        if (!destColorSpace) {
            dbgFile << "Unsupported colorspace" << pd->colorSpace()->name();
            return ImportExportCodes::FormatColorSpaceUnsupported;
        }
        pd.attach(new KisPaintDevice(*pd));
        pd->convertTo(destColorSpace);
    }

    // Save depth
    quint16 depth = static_cast<quint16>(8 * pd->pixelSize() / pd->channelCount());
    TIFFSetField(image(), TIFFTAG_BITSPERSAMPLE, depth);
    // Save number of samples
    quint16 nbchannels;
    if (m_options->alpha) {
        nbchannels = static_cast<quint16>(pd->channelCount());
        TIFFSetField(image(), TIFFTAG_SAMPLESPERPIXEL, nbchannels);
        uint16_t sampleinfo[1] = {EXTRASAMPLE_UNASSALPHA};
        TIFFSetField(image(), TIFFTAG_EXTRASAMPLES, 1, sampleinfo);
    } else {
        nbchannels = static_cast<quint16>(pd->channelCount() - 1);
        TIFFSetField(image(), TIFFTAG_SAMPLESPERPIXEL, nbchannels);
        TIFFSetField(image(), TIFFTAG_EXTRASAMPLES, 0);
    }

    // Save colorspace information
    TIFFSetField(image(), TIFFTAG_PHOTOMETRIC, color_type);
    TIFFSetField(image(), TIFFTAG_SAMPLEFORMAT, sample_format);
    TIFFSetField(image(), TIFFTAG_IMAGEWIDTH, layer->image()->width());
    TIFFSetField(image(), TIFFTAG_IMAGELENGTH, layer->image()->height());

    // Set the compression options
    TIFFSetField(image(), TIFFTAG_COMPRESSION, m_options->compressionType);
    TIFFSetField(image(), TIFFTAG_JPEGQUALITY, m_options->jpegQuality);
    TIFFSetField(image(), TIFFTAG_ZIPQUALITY, m_options->deflateCompress);
    TIFFSetField(image(), TIFFTAG_PIXARLOGQUALITY, m_options->pixarLogCompress);

    // Set the predictor
    TIFFSetField(image(), TIFFTAG_PREDICTOR, m_options->predictor);

    // Use contiguous configuration
    TIFFSetField(image(), TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    // Use 8 rows per strip
    TIFFSetField(image(), TIFFTAG_ROWSPERSTRIP, 8);

    // Save profile
    if (m_options->saveProfile) {
        const KoColorProfile *profile = pd->colorSpace()->profile();
        if (profile && profile->type() == "icc" && !profile->rawData().isEmpty()) {
            QByteArray ba = profile->rawData();
            TIFFSetField(image(), TIFFTAG_ICCPROFILE, ba.size(), ba.constData());
        }
    }
    tsize_t stripsize = TIFFStripSize(image());
    tdata_t buff = _TIFFmalloc(stripsize);
    qint32 height = layer->image()->height();
    qint32 width = layer->image()->width();
    bool r = true;
    for (qint32 y = 0; y < height; y++) {
        KisHLineConstIteratorSP it = pd->createHLineConstIteratorNG(0, y, width);
        switch (color_type) {
        case PHOTOMETRIC_MINISBLACK: {
            quint8 poses[] = {0, 1};
            r = copyDataToStrips(it, buff, depth, sample_format, 1, poses);
        } break;
        case PHOTOMETRIC_RGB: {
            quint8 poses[4];
            if (sample_format == SAMPLEFORMAT_IEEEFP) {
                poses[2] = 2;
                poses[1] = 1;
                poses[0] = 0;
                poses[3] = 3;
            } else {
                poses[0] = 2;
                poses[1] = 1;
                poses[2] = 0;
                poses[3] = 3;
            }
            r = copyDataToStrips(it, buff, depth, sample_format, 3, poses);
        } break;
        case PHOTOMETRIC_SEPARATED: {
            quint8 poses[] = {0, 1, 2, 3, 4};
            r = copyDataToStrips(it, buff, depth, sample_format, 4, poses);
        } break;
        case PHOTOMETRIC_ICCLAB: {
            quint8 poses[] = {0, 1, 2, 3};
            r = copyDataToStrips(it, buff, depth, sample_format, 3, poses);
        } break;
            return ImportExportCodes::FormatColorSpaceUnsupported;
        }
        if (!r)
            return ImportExportCodes::InternalError;
        TIFFWriteScanline(image(), buff, static_cast<quint32>(y), (tsample_t)-1);
    }
    _TIFFfree(buff);

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
            KisTiffPsdLayerRecord layerSection(TIFFIsBigEndian(image()),
                                               static_cast<uint32_t>(width),
                                               static_cast<uint32_t>(height),
                                               static_cast<uint16_t>(depth),
                                               nbchannels,
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
                PSDResourceBlock *resourceBlock = dynamic_cast<PSDResourceBlock *>(annotation.data());
                if (resourceBlock) {
                    dbgFile << "Adding PSD Resource Block" << resourceBlock->identifier;
                    resourceSection.resources[(KisTiffPsdResourceRecord::PSDResourceID)resourceBlock->identifier] = resourceBlock;
                }
            }
        }

        // Add resolution block
        {
            RESN_INFO_1005 *resInfo = new RESN_INFO_1005();
            resInfo->hRes = static_cast<int>(INCH_TO_POINT(layer->image()->xRes()));
            resInfo->vRes = static_cast<int>(INCH_TO_POINT(layer->image()->yRes()));
            PSDResourceBlock *block = new PSDResourceBlock();
            block->identifier = KisTiffPsdResourceRecord::RESN_INFO;
            block->resource = resInfo;
            resourceSection.resources[KisTiffPsdResourceRecord::RESN_INFO] = block;
        }

        // Add icc block
        {
            ICC_PROFILE_1039 *profileInfo = new ICC_PROFILE_1039();
            profileInfo->icc = layer->image()->profile()->rawData();
            PSDResourceBlock *block = new PSDResourceBlock();
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
