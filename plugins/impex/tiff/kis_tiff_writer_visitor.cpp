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

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

namespace
{
    bool writeColorSpaceInformation(TIFF* image, const KoColorSpace * cs, uint16& color_type, uint16& sample_format)
    {
        dbgKrita << cs->id();
        if (cs->id() == "GRAYA" || cs->id() == "GRAYAU16") {
            color_type = PHOTOMETRIC_MINISBLACK;
            return true;
        }
        if (KoID(cs->id()) == KoID("RGBA") || KoID(cs->id()) == KoID("RGBA16")) {
            color_type = PHOTOMETRIC_RGB;
            return true;
        }
       if (KoID(cs->id()) == KoID("RGBAF16") || KoID(cs->id()) == KoID("RGBAF32")) {
           color_type = PHOTOMETRIC_RGB;
           sample_format = SAMPLEFORMAT_IEEEFP;
           return true;
       }
        if (cs->id() == "CMYK" || cs->id() == "CMYKAU16") {
            color_type = PHOTOMETRIC_SEPARATED;
            TIFFSetField(image, TIFFTAG_INKSET, INKSET_CMYK);
            return true;
        }
        if (cs->id() == "LABA") {
            color_type = PHOTOMETRIC_ICCLAB;
            return true;
        }
        return false;

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
    uint16 color_type;
    uint16 sample_format = SAMPLEFORMAT_UINT;
    if (!writeColorSpaceInformation(image(), pd->colorSpace(), color_type, sample_format)) { // unsupported colorspace
        return false;
    }
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
