/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_writer_visitor.h"

#include <QBuffer>

#include <tiff.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoID.h>
#include <kis_meta_data_backend_registry.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

namespace
{
    bool isBitDepthFloat(QString depth) {
        return depth.contains("F");
    }

    bool writeColorSpaceInformation(TIFF* image, const KoColorSpace * cs, uint16_t& color_type, uint16_t& sample_format, const KoColorSpace* &destColorSpace)
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
            const QString profile = "sRGB-elle-V2-srgbtrc";
            destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), depth, profile);
            if (isBitDepthFloat(depth)) {
                sample_format = SAMPLEFORMAT_IEEEFP;
            }
            return false;
        }

    }
}

KisTIFFWriterVisitor::KisTIFFWriterVisitor(TIFF*image, KisTIFFOptions* options)
    : m_image(image)
    , m_options(options)
{
}

KisTIFFWriterVisitor::~KisTIFFWriterVisitor()
{
}

bool KisTIFFWriterVisitor::copyDataToStrips(KisHLineConstIteratorSP it, tdata_t buff, uint8_t depth, uint16_t sample_format, uint8_t nbcolorssamples, quint8* poses)
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
            if (m_options->alpha) *(dst++) = d[poses[i]];
        } while (it->nextPixel());
        return true;
    }
    else if (depth == 16 ) {
        if (sample_format == SAMPLEFORMAT_IEEEFP) {
#ifdef HAVE_OPENEXR
            half *dst = reinterpret_cast<half *>(buff);
            do {
                const half *d = reinterpret_cast<const half *>(it->oldRawData());
                int i;
                for (i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses[i]];
                }
                if (m_options->alpha) *(dst++) = d[poses[i]];

            } while (it->nextPixel());
            return true;
#endif
        }
        else {
            quint16 *dst = reinterpret_cast<quint16 *>(buff);
            do {
                const quint16 *d = reinterpret_cast<const quint16 *>(it->oldRawData());
                int i;
                for (i = 0; i < nbcolorssamples; i++) {
                    *(dst++) = d[poses[i]];
                }
                if (m_options->alpha) *(dst++) = d[poses[i]];

            } while (it->nextPixel());
            return true;
        }
    }
    else if (depth == 8) {
        quint8 *dst = reinterpret_cast<quint8 *>(buff);
        do {
            const quint8 *d = it->oldRawData();
            int i;
            for (i = 0; i < nbcolorssamples; i++) {
                *(dst++) = d[poses[i]];
            }
            if (m_options->alpha) *(dst++) = d[poses[i]];

        } while (it->nextPixel());
        return true;
    }
    return false;
}

bool KisTIFFWriterVisitor::saveLayerProjection(KisLayer *layer)
{
    dbgFile << "visiting on layer" << layer->name() << "";
    KisPaintDeviceSP pd = layer->projection();

    uint16_t color_type;
    uint16_t sample_format = SAMPLEFORMAT_UINT;
    const KoColorSpace* destColorSpace;
    // Check colorspace
    if (!writeColorSpaceInformation(image(), pd->colorSpace(), color_type, sample_format, destColorSpace)) { // unsupported colorspace
        if (!destColorSpace) {
            return false;
        }
        pd.attach(new KisPaintDevice(*pd));
        pd->convertTo(destColorSpace);
    }

    // Save depth
    int depth = 8 * pd->pixelSize() / pd->channelCount();
    TIFFSetField(image(), TIFFTAG_BITSPERSAMPLE, depth);
    // Save number of samples
    if (m_options->alpha) {
        TIFFSetField(image(), TIFFTAG_SAMPLESPERPIXEL, pd->channelCount());
        uint16_t sampleinfo[1] = { EXTRASAMPLE_UNASSALPHA };
        TIFFSetField(image(), TIFFTAG_EXTRASAMPLES, 1, sampleinfo);
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
        const KoColorProfile* profile = pd->colorSpace()->profile();
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
    for (int y = 0; y < height; y++) {
        KisHLineConstIteratorSP it = pd->createHLineConstIteratorNG(0, y, width);
        switch (color_type) {
        case PHOTOMETRIC_MINISBLACK: {
                quint8 poses[] = { 0, 1 };
                r = copyDataToStrips(it, buff, depth, sample_format, 1, poses);
            }
            break;
        case PHOTOMETRIC_RGB: {
                quint8 poses[4];
                if (sample_format == SAMPLEFORMAT_IEEEFP) {
                    poses[2] = 2; poses[1] = 1; poses[0] = 0; poses[3] = 3;
                } else {
                    poses[0] = 2; poses[1] = 1; poses[2] = 0; poses[3] = 3;
                }
                r = copyDataToStrips(it, buff, depth, sample_format, 3, poses);
            }
            break;
        case PHOTOMETRIC_SEPARATED: {
                quint8 poses[] = { 0, 1, 2, 3, 4 };
                r = copyDataToStrips(it, buff, depth, sample_format, 4, poses);
            }
            break;
        case PHOTOMETRIC_ICCLAB: {
                quint8 poses[] = { 0, 1, 2, 3 };
                r = copyDataToStrips(it, buff, depth, sample_format, 3, poses);
            }
            break;
            return false;
        }
        if (!r) return false;
        TIFFWriteScanline(image(), buff, y, (tsample_t) - 1);
    }
    _TIFFfree(buff);

    {
        // IPTC
        KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("iptc");
        QBuffer buf;
        io->saveTo(layer->metaData(), &buf, KisMetaData::IOBackend::NoHeader);

        if (!TIFFSetField(image(), TIFFTAG_RICHTIFFIPTC, static_cast<uint32_t>(buf.size()), buf.data().data())) {
            dbgFile << "Failed to write the IPTC metadata to the TIFF field";
        }
    }

    {
        // XMP
        KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("xmp");
        QBuffer buf;
        io->saveTo(layer->metaData(), &buf, KisMetaData::IOBackend::NoHeader);

        if (!TIFFSetField(image(), TIFFTAG_XMLPACKET, static_cast<uint32_t>(buf.size()), buf.data().data())) {
            dbgFile << "Failed to write the XMP metadata to the TIFF field";
        }
    }
    TIFFWriteDirectory(image());
    return true;
}
