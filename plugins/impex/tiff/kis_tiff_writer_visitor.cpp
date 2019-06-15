/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tiff_writer_visitor.h"

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoColorSpaceRegistry.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

namespace
{
    bool isBitDepthFloat(QString depth) {
        return depth.contains("F");
    }

    bool writeColorSpaceInformation(TIFF* image, const KoColorSpace * cs, uint16& color_type, uint16& sample_format, const KoColorSpace* &destColorSpace)
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
                destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id(), cs->profile());
                return false;
            }
            return true;

        } else if (id.contains("LABA")) {
            color_type = PHOTOMETRIC_ICCLAB;
            if (depth != "U16") {
                destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), cs->profile());
                return false;
            }
            return true;

        } else if (id.contains("GRAYA")) {
            color_type = PHOTOMETRIC_MINISBLACK;
            QList<QString> possibleDepths;
            possibleDepths << "U16" << "U8";
            if (!possibleDepths.contains(depth)) {
                destColorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), cs->profile());
                return false;
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

bool KisTIFFWriterVisitor::copyDataToStrips(KisHLineConstIteratorSP it, tdata_t buff, uint8 depth, uint16 sample_format, uint8 nbcolorssamples, quint8* poses)
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

    uint16 color_type;
    uint16 sample_format = SAMPLEFORMAT_UINT;
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
        uint16 sampleinfo[1] = { EXTRASAMPLE_UNASSALPHA };
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
    TIFFWriteDirectory(image());
    return true;
}
