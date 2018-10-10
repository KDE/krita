/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tiff_converter.h"

#include <stdio.h>

#include <QFile>
#include <QApplication>

#include <QFileInfo>

#include <KoDocumentInfo.h>
#include <KoUnit.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <KoColorProfile.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>

#include "kis_tiff_reader.h"
#include "kis_tiff_ycbcr_reader.h"
#include "kis_buffer_stream.h"
#include "kis_tiff_writer_visitor.h"

#if TIFFLIB_VERSION < 20111221
typedef size_t tmsize_t;
#endif

namespace
{

QPair<QString, QString> getColorSpaceForColorType(uint16 sampletype, uint16 color_type, uint16 color_nb_bits, TIFF *image, uint16 &nbchannels, uint16 &extrasamplescount, uint8 &destDepth)
{
    if (color_type == PHOTOMETRIC_MINISWHITE || color_type == PHOTOMETRIC_MINISBLACK) {
        if (nbchannels == 0) nbchannels = 1;
        extrasamplescount = nbchannels - 1; // FIX the extrasamples count in case of
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
           if (color_nb_bits == 16) {
               destDepth = 16;
               return QPair<QString, QString>(GrayAColorModelID.id(), Float16BitsColorDepthID.id());
           }
           else if (color_nb_bits == 32) {
               destDepth = 32;
               return QPair<QString, QString>(GrayAColorModelID.id(), Float32BitsColorDepthID.id());
           }
        }
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return QPair<QString, QString>(GrayAColorModelID.id(), Integer8BitsColorDepthID.id());
        }
        else {
            destDepth = 16;
            return QPair<QString, QString>(GrayAColorModelID.id(), Integer16BitsColorDepthID.id());
        }

    } else if (color_type == PHOTOMETRIC_RGB  /*|| color_type == */) {
        if (nbchannels == 0) nbchannels = 3;
        extrasamplescount = nbchannels - 3; // FIX the extrasamples count in case of
        if (sampletype == SAMPLEFORMAT_IEEEFP) {
            if (color_nb_bits == 16) {
                destDepth = 16;
                return QPair<QString, QString>(RGBAColorModelID.id(), Float16BitsColorDepthID.id());
            }
            else if (color_nb_bits == 32) {
                destDepth = 32;
                return QPair<QString, QString>(RGBAColorModelID.id(), Float32BitsColorDepthID.id());
            }
            return QPair<QString, QString>();
        }
        else {
            if (color_nb_bits <= 8) {
                destDepth = 8;
                return QPair<QString, QString>(RGBAColorModelID.id(), Integer8BitsColorDepthID.id());
            }
            else {
                destDepth = 16;
                return QPair<QString, QString>(RGBAColorModelID.id(), Integer16BitsColorDepthID.id());
            }
        }
    } else if (color_type == PHOTOMETRIC_YCBCR) {
        if (nbchannels == 0) nbchannels = 3;
        extrasamplescount = nbchannels - 3; // FIX the extrasamples count in case of
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return QPair<QString, QString>(YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id());
        }
        else {
            destDepth = 16;
            return QPair<QString, QString>(YCbCrAColorModelID.id(), Integer16BitsColorDepthID.id());
        }
    }
    else if (color_type == PHOTOMETRIC_SEPARATED) {
        if (nbchannels == 0) nbchannels = 4;
        // SEPARATED is in general CMYK but not always, so we check
        uint16 inkset;
        if ((TIFFGetField(image, TIFFTAG_INKSET, &inkset) == 0)) {
            dbgFile << "Image does not define the inkset.";
            inkset = 2;
        }
        if (inkset !=  INKSET_CMYK) {
            dbgFile << "Unsupported inkset (right now, only CMYK is supported)";
            char** ink_names;
            uint16 numberofinks;
            if (TIFFGetField(image, TIFFTAG_INKNAMES, &ink_names)  == 1 && TIFFGetField(image, TIFFTAG_NUMBEROFINKS, &numberofinks)  == 1) {
                dbgFile << "Inks are :";
                for (uint i = 0; i < numberofinks; i++) {
                    dbgFile << ink_names[i];
                }
            }
            else {
                dbgFile << "inknames are not defined !";
                // To be able to read stupid adobe files, if there are no information about inks and four channels, then it's a CMYK file :
                if (nbchannels - extrasamplescount != 4) {
                    return QPair<QString, QString>();
                }
            }
        }
        if (color_nb_bits <= 8) {
            destDepth = 8;
            return QPair<QString, QString>(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());
        }
        else {
            destDepth = 16;
            return QPair<QString, QString>(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id());
        }
    }
    else if (color_type == PHOTOMETRIC_CIELAB || color_type == PHOTOMETRIC_ICCLAB) {
        destDepth = 16;
        if (nbchannels == 0) nbchannels = 3;
        extrasamplescount = nbchannels - 3; // FIX the extrasamples count
        return QPair<QString, QString>(LABAColorModelID.id(), Integer16BitsColorDepthID.id());
    }
    else if (color_type ==  PHOTOMETRIC_PALETTE) {
        destDepth = 16;
        if (nbchannels == 0) nbchannels = 2;
        extrasamplescount = nbchannels - 2; // FIX the extrasamples count
        // <-- we will convert the index image to RGBA16 as the palette is always on 16bits colors
        return QPair<QString, QString>(RGBAColorModelID.id(), Integer16BitsColorDepthID.id());
    }
    return QPair<QString, QString>();
}
}

KisPropertiesConfigurationSP KisTIFFOptions::toProperties() const
{
    QHash<int, int> compToIndex;
    compToIndex[COMPRESSION_NONE] = 0;
    compToIndex[COMPRESSION_JPEG] = 1;
    compToIndex[COMPRESSION_DEFLATE] = 2;
    compToIndex[COMPRESSION_LZW] = 3;
    compToIndex[COMPRESSION_PIXARLOG] = 8;

    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    cfg->setProperty("compressiontype", compToIndex.value(compressionType, 0));
    cfg->setProperty("predictor", predictor - 1);
    cfg->setProperty("alpha", alpha);
    cfg->setProperty("flatten", flatten);
    cfg->setProperty("quality", jpegQuality);
    cfg->setProperty("deflate", deflateCompress);
    cfg->setProperty("pixarlog", pixarLogCompress);
    cfg->setProperty("saveProfile", saveProfile);

    return cfg;
}

void KisTIFFOptions::fromProperties(KisPropertiesConfigurationSP cfg)
{
    QHash<int, int> indexToComp;
    indexToComp[0] = COMPRESSION_NONE;
    indexToComp[1] = COMPRESSION_JPEG;
    indexToComp[2] = COMPRESSION_DEFLATE;
    indexToComp[3] = COMPRESSION_LZW;
    indexToComp[4] = COMPRESSION_PIXARLOG;

    // old value that might be still stored in a config (remove after Krita 5.0 :) )
    indexToComp[8] = COMPRESSION_PIXARLOG;

    compressionType =
        indexToComp.value(
            cfg->getInt("compressiontype", 0),
            COMPRESSION_NONE);

    predictor = cfg->getInt("predictor", 0) + 1;
    alpha = cfg->getBool("alpha", true);
    flatten = cfg->getBool("flatten", true);
    jpegQuality = cfg->getInt("quality", 80);
    deflateCompress = cfg->getInt("deflate", 6);
    pixarLogCompress = cfg->getInt("pixarlog", 6);
    saveProfile = cfg->getBool("saveProfile", true);
}


KisTIFFConverter::KisTIFFConverter(KisDocument *doc)
{
    m_doc = doc;
    m_stop = false;

    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);
}

KisTIFFConverter::~KisTIFFConverter()
{
}

KisImageBuilder_Result KisTIFFConverter::decode(const QString &filename)
{
    dbgFile << "Start decoding TIFF File";
    // Opent the TIFF file
    TIFF *image = 0;
    if ((image = TIFFOpen(QFile::encodeName(filename), "r")) == 0) {
        dbgFile << "Could not open the file, either it does not exist, either it is not a TIFF :" << filename;
        return (KisImageBuilder_RESULT_BAD_FETCH);
    }
    do {
        dbgFile << "Read new sub-image";
        KisImageBuilder_Result result = readTIFFDirectory(image);
        if (result != KisImageBuilder_RESULT_OK) {
            return result;
        }
    } while (TIFFReadDirectory(image));
    // Freeing memory
    TIFFClose(image);
    return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisTIFFConverter::readTIFFDirectory(TIFF* image)
{
    // Read information about the tiff
    uint32 width, height;
    if (TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width) == 0) {
        dbgFile << "Image does not define its width";
        TIFFClose(image);
        return KisImageBuilder_RESULT_INVALID_ARG;
    }

    if (TIFFGetField(image, TIFFTAG_IMAGELENGTH, &height) == 0) {
        dbgFile << "Image does not define its height";
        TIFFClose(image);
        return KisImageBuilder_RESULT_INVALID_ARG;
    }

    float xres;
    if (TIFFGetField(image, TIFFTAG_XRESOLUTION, &xres) == 0) {
        dbgFile << "Image does not define x resolution";
        // but we don't stop
        xres = 100;
    }

    float yres;
    if (TIFFGetField(image, TIFFTAG_YRESOLUTION, &yres) == 0) {
        dbgFile << "Image does not define y resolution";
        // but we don't stop
        yres = 100;
    }

    uint16 depth;
    if ((TIFFGetField(image, TIFFTAG_BITSPERSAMPLE, &depth) == 0)) {
        dbgFile << "Image does not define its depth";
        depth = 1;
    }

    uint16 sampletype;
    if ((TIFFGetField(image, TIFFTAG_SAMPLEFORMAT, &sampletype) == 0)) {
        dbgFile << "Image does not define its sample type";
        sampletype =  SAMPLEFORMAT_UINT;
    }

    // Determine the number of channels (useful to know if a file has an alpha or not
    uint16 nbchannels;
    if (TIFFGetField(image, TIFFTAG_SAMPLESPERPIXEL, &nbchannels) == 0) {
        dbgFile << "Image has an undefined number of samples per pixel";
        nbchannels = 0;
    }

    // Get the number of extrasamples and information about them
    uint16 *sampleinfo = 0, extrasamplescount;
    if (TIFFGetField(image, TIFFTAG_EXTRASAMPLES, &extrasamplescount, &sampleinfo) == 0) {
        extrasamplescount = 0;
    }

    // Determine the colorspace
    uint16 color_type;
    if (TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &color_type) == 0) {
        dbgFile << "Image has an undefined photometric interpretation";
        color_type = PHOTOMETRIC_MINISWHITE;
    }

    uint8 dstDepth = 0;
    QPair<QString, QString> colorSpaceIdTag = getColorSpaceForColorType(sampletype, color_type, depth, image, nbchannels, extrasamplescount, dstDepth);
    if (colorSpaceIdTag.first.isEmpty()) {
        dbgFile << "Image has an unsupported colorspace :" << color_type << " for this depth :" << depth;
        TIFFClose(image);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    dbgFile << "Colorspace is :" << colorSpaceIdTag.first << colorSpaceIdTag.second << " with a depth of" << depth << " and with a nb of channels of" << nbchannels;

    // Read image profile
    dbgFile << "Reading profile";
    const KoColorProfile* profile = 0;
    quint32 EmbedLen;
    quint8* EmbedBuffer;

    if (TIFFGetField(image, TIFFTAG_ICCPROFILE, &EmbedLen, &EmbedBuffer) == 1) {
        dbgFile << "Profile found";
        QByteArray rawdata;
        rawdata.resize(EmbedLen);
        memcpy(rawdata.data(), EmbedBuffer, EmbedLen);
        profile = KoColorSpaceRegistry::instance()->createColorProfile(colorSpaceIdTag.first, colorSpaceIdTag.second, rawdata);
    }

    const QString colorSpaceId =
        KoColorSpaceRegistry::instance()->colorSpaceId(colorSpaceIdTag.first, colorSpaceIdTag.second);

    // Check that the profile is used by the color space
    if (profile && !KoColorSpaceRegistry::instance()->profileIsCompatible(profile, colorSpaceId)) {
        dbgFile << "The profile " << profile->name() << " is not compatible with the color space model " << colorSpaceIdTag.first << " " << colorSpaceIdTag.second;
        profile = 0;
    }

    // Do not use the linear gamma profile for 16 bits/channel by default, tiff files are usually created with
    // gamma correction. XXX: Should we ask the user?
    if (!profile) {
        if (colorSpaceIdTag.first == RGBAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName("sRGB-elle-V2-srgbtrc.icc");
        } else if (colorSpaceIdTag.first == GrayAColorModelID.id()) {
            profile = KoColorSpaceRegistry::instance()->profileByName("Gray-D50-elle-V2-srgbtrc.icc");
        }
    }

    // Retrieve a pointer to the colorspace
    const KoColorSpace* cs = 0;
    if (profile && profile->isSuitableForOutput()) {
        dbgFile << "image has embedded profile:" << profile -> name() << "";
        cs = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceIdTag.first, colorSpaceIdTag.second, profile);
    }
    else {
        cs = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceIdTag.first, colorSpaceIdTag.second, 0);
    }

    if (cs == 0) {
        dbgFile << "Colorspace" << colorSpaceIdTag.first << colorSpaceIdTag.second << " is not available, please check your installation.";
        TIFFClose(image);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }

    // Create the cmsTransform if needed
    KoColorTransformation* transform = 0;
    if (profile && !profile->isSuitableForOutput()) {
        dbgFile << "The profile can't be used in krita, need conversion";
        transform = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceIdTag.first, colorSpaceIdTag.second, profile)->createColorConverter(cs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    }

    // Check if there is an alpha channel
    int8 alphapos = -1; // <- no alpha
    // Check which extra is alpha if any
    dbgFile << "There are" << nbchannels << " channels and" << extrasamplescount << " extra channels";
    if (sampleinfo) { // index images don't have any sampleinfo, and therefore sampleinfo == 0
        for (int i = 0; i < extrasamplescount; i ++) {
            dbgFile << "sample" << i << "extra sample count" << extrasamplescount << "color channel count" << (cs->colorChannelCount()) << "Number of channels" <<  nbchannels << "sample info" << sampleinfo[i];
            if (sampleinfo[i] == EXTRASAMPLE_UNSPECIFIED) {
                qWarning() << "Extra sample type not defined for this file, assuming unassociated alpha.";
                alphapos = i;
            }
            if (sampleinfo[i] == EXTRASAMPLE_ASSOCALPHA) {
                // XXX: dangelo: the color values are already multiplied with
                // the alpha value.  This needs to be reversed later (postprocessor?)
                qWarning() << "Associated alpha in this file: krita does not handle plremultiplied alpha.";
                alphapos = i;
            }

            if (sampleinfo[i] == EXTRASAMPLE_UNASSALPHA) {
                // color values are not premultiplied with alpha, and can be used as they are.
                alphapos = i;
            }
        }
    }

    dbgFile << "Alpha pos:" << alphapos;

    // Read META Information
    KoDocumentInfo * info = m_doc->documentInfo();
    char* text;
    if (TIFFGetField(image, TIFFTAG_ARTIST, &text) == 1) {
        info->setAuthorInfo("creator", text);
    }
    if (TIFFGetField(image, TIFFTAG_DOCUMENTNAME, &text) == 1) {
        info->setAboutInfo("title", text);
    }
    if (TIFFGetField(image, TIFFTAG_IMAGEDESCRIPTION, &text)  == 1) {
        info->setAboutInfo("description", text);
    }


    // Get the planar configuration
    uint16 planarconfig;
    if (TIFFGetField(image, TIFFTAG_PLANARCONFIG, &planarconfig) == 0) {
        dbgFile << "Plannar configuration is not define";
        TIFFClose(image);
        return KisImageBuilder_RESULT_INVALID_ARG;
    }
    // Creating the KisImageSP
    if (! m_image) {
        m_image = new KisImage(m_doc->createUndoStore(), width, height, cs, "built image");
        m_image->setResolution( POINT_TO_INCH(xres), POINT_TO_INCH(yres )); // It is the "invert" macro because we convert from pointer-per-inchs to points
        Q_CHECK_PTR(m_image);
    }
    else {
        if (m_image->width() < (qint32)width || m_image->height() < (qint32)height) {
            quint32 newwidth = (m_image->width() < (qint32)width) ? width : m_image->width();
            quint32 newheight = (m_image->height() < (qint32)height) ? height : m_image->height();
            m_image->resizeImage(QRect(0,0,newwidth, newheight));
        }
    }
    KisPaintLayer* layer = new KisPaintLayer(m_image.data(), m_image -> nextLayerName(), quint8_MAX);
    tdata_t buf = 0;
    tdata_t* ps_buf = 0; // used only for planar configuration separated
    KisBufferStreamBase* tiffstream;

    KisTIFFReaderBase* tiffReader = 0;

    quint8 poses[5];
    KisTIFFPostProcessor* postprocessor = 0;

    // Configure poses
    uint8 nbcolorsamples = nbchannels - extrasamplescount;
    switch (color_type) {
    case PHOTOMETRIC_MINISWHITE: {
        poses[0] = 0; poses[1] = 1;
        postprocessor = new KisTIFFPostProcessorInvert(nbcolorsamples);
    }
        break;
    case PHOTOMETRIC_MINISBLACK: {
        poses[0] = 0; poses[1] = 1;
        postprocessor = new KisTIFFPostProcessor(nbcolorsamples);
    }
        break;
    case PHOTOMETRIC_CIELAB: {
        poses[0] = 0; poses[1] = 1; poses[2] = 2; poses[3] = 3;
        postprocessor = new KisTIFFPostProcessorCIELABtoICCLAB(nbcolorsamples);
    }
        break;
    case PHOTOMETRIC_ICCLAB: {
        poses[0] = 0; poses[1] = 1; poses[2] = 2; poses[3] = 3;
        postprocessor = new KisTIFFPostProcessor(nbcolorsamples);
    }
        break;
    case PHOTOMETRIC_RGB: {
        if (sampletype == SAMPLEFORMAT_IEEEFP)
        {
            poses[2] = 2; poses[1] = 1; poses[0] = 0; poses[3] = 3;
        } else {
            poses[0] = 2; poses[1] = 1; poses[2] = 0; poses[3] = 3;
        }
        postprocessor = new KisTIFFPostProcessor(nbcolorsamples);
    }
        break;
    case PHOTOMETRIC_SEPARATED: {
        poses[0] = 0; poses[1] = 1; poses[2] = 2; poses[3] = 3; poses[4] = 4;
        postprocessor = new KisTIFFPostProcessor(nbcolorsamples);
    }
        break;
    default:
        break;
    }


    // Initisalize tiffReader
    uint16 * lineSizeCoeffs = new uint16[nbchannels];
    uint16 vsubsampling = 1;
    uint16 hsubsampling = 1;
    for (uint i = 0; i < nbchannels; i++) {
        lineSizeCoeffs[i] = 1;
    }
    if (color_type == PHOTOMETRIC_PALETTE) {
        uint16 *red; // No need to free them they are free by libtiff
        uint16 *green;
        uint16 *blue;
        if ((TIFFGetField(image, TIFFTAG_COLORMAP, &red, &green, &blue)) == 0) {
            dbgFile << "Indexed image does not define a palette";
            TIFFClose(image);
            delete [] lineSizeCoeffs;
            return KisImageBuilder_RESULT_INVALID_ARG;
        }

        tiffReader = new KisTIFFReaderFromPalette(layer->paintDevice(), red, green, blue, poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor);
    } else if (color_type == PHOTOMETRIC_YCBCR) {
        TIFFGetFieldDefaulted(image, TIFFTAG_YCBCRSUBSAMPLING, &hsubsampling, &vsubsampling);
        lineSizeCoeffs[1] = hsubsampling;
        lineSizeCoeffs[2] = hsubsampling;
        uint16 position;
        TIFFGetFieldDefaulted(image, TIFFTAG_YCBCRPOSITIONING, &position);
        if (dstDepth == 8) {
            tiffReader = new KisTIFFYCbCrReaderTarget8Bit(layer->paintDevice(), layer->image()->width(), layer->image()->height(), poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor, hsubsampling, vsubsampling);
        }
        else if (dstDepth == 16) {
            tiffReader = new KisTIFFYCbCrReaderTarget16Bit(layer->paintDevice(), layer->image()->width(), layer->image()->height(), poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor, hsubsampling, vsubsampling);
        }
    }
    else if (dstDepth == 8) {
        tiffReader = new KisTIFFReaderTarget8bit(layer->paintDevice(), poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor);
    }
    else if (dstDepth == 16) {
        uint16 alphaValue;
        if (sampletype == SAMPLEFORMAT_IEEEFP)
        {
          alphaValue = 15360; // representation of 1.0 in half
        } else {
          alphaValue = quint16_MAX;
        }
        tiffReader = new KisTIFFReaderTarget16bit(layer->paintDevice(), poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor, alphaValue);
    }
    else if (dstDepth == 32) {
        union {
          float f;
          uint32 i;
        } alphaValue;
        if (sampletype == SAMPLEFORMAT_IEEEFP)
        {
          alphaValue.f = 1.0f;
        } else {
          alphaValue.i = quint32_MAX;
        }
        tiffReader = new KisTIFFReaderTarget32bit(layer->paintDevice(), poses, alphapos, depth, sampletype, nbcolorsamples, extrasamplescount, transform, postprocessor, alphaValue.i);
    }

    if (!tiffReader) {
        delete postprocessor;
        delete[] lineSizeCoeffs;
        TIFFClose(image);
        dbgFile << "Image has an invalid/unsupported color type: " << color_type;
        return KisImageBuilder_RESULT_INVALID_ARG;
    }

    if (TIFFIsTiled(image)) {
        dbgFile << "tiled image";
        uint32 tileWidth, tileHeight;
        uint32 x, y;
        TIFFGetField(image, TIFFTAG_TILEWIDTH, &tileWidth);
        TIFFGetField(image, TIFFTAG_TILELENGTH, &tileHeight);
        uint32 linewidth = (tileWidth * depth * nbchannels) / 8;
        if (planarconfig == PLANARCONFIG_CONTIG) {
            buf = _TIFFmalloc(TIFFTileSize(image));
            if (depth < 16) {
                tiffstream = new KisBufferStreamContigBelow16((uint8*)buf, depth, linewidth);
            }
            else if (depth < 32) {
                tiffstream = new KisBufferStreamContigBelow32((uint8*)buf, depth, linewidth);
            }
            else {
                tiffstream = new KisBufferStreamContigAbove32((uint8*)buf, depth, linewidth);
            }
        }
        else {
            ps_buf = new tdata_t[nbchannels];
            uint32 * lineSizes = new uint32[nbchannels];
            tmsize_t baseSize = TIFFTileSize(image) / nbchannels;
            for (uint i = 0; i < nbchannels; i++) {
                ps_buf[i] = _TIFFmalloc(baseSize);
                lineSizes[i] = tileWidth; // baseSize / lineSizeCoeffs[i];
            }
            tiffstream = new KisBufferStreamSeperate((uint8**) ps_buf, nbchannels, depth, lineSizes);
            delete [] lineSizes;
        }
        dbgFile << linewidth << "" << nbchannels << "" << layer->paintDevice()->colorSpace()->colorChannelCount();
        for (y = 0; y < height; y += tileHeight) {
            for (x = 0; x < width; x += tileWidth) {
                dbgFile << "Reading tile x =" << x << " y =" << y;
                if (planarconfig == PLANARCONFIG_CONTIG) {
                    TIFFReadTile(image, buf, x, y, 0, (tsample_t) - 1);
                }
                else {
                    for (uint i = 0; i < nbchannels; i++) {
                        TIFFReadTile(image, ps_buf[i], x, y, 0, i);
                    }
                }
                uint32 realTileWidth = (x + tileWidth) < width ? tileWidth : width - x;
                for (uint yintile = 0; y + yintile < height && yintile < tileHeight / vsubsampling;) {
                    tiffReader->copyDataToChannels(x, y + yintile , realTileWidth, tiffstream);
                    yintile += 1;
                    tiffstream->moveToLine(yintile);
                }
                tiffstream->restart();
            }
        }
    }
    else {
        dbgFile << "striped image";
        tsize_t stripsize = TIFFStripSize(image);
        uint32 rowsPerStrip;
        TIFFGetFieldDefaulted(image, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);
        dbgFile << rowsPerStrip << "" << height;
        rowsPerStrip = qMin(rowsPerStrip, height); // when TIFFNumberOfStrips(image) == 1 it might happen that rowsPerStrip is incorrectly set
        if (planarconfig == PLANARCONFIG_CONTIG) {
            buf = _TIFFmalloc(stripsize);
            if (depth < 16) {
                tiffstream = new KisBufferStreamContigBelow16((uint8*)buf, depth, stripsize / rowsPerStrip);
            }
            else if (depth < 32) {
                tiffstream = new KisBufferStreamContigBelow32((uint8*)buf, depth, stripsize / rowsPerStrip);
            }
            else {
                tiffstream = new KisBufferStreamContigAbove32((uint8*)buf, depth, stripsize / rowsPerStrip);
            }
        }
        else {
            ps_buf = new tdata_t[nbchannels];
            uint32 scanLineSize = stripsize / rowsPerStrip;
            dbgFile << " scanLineSize for each plan =" << scanLineSize;
            uint32 * lineSizes = new uint32[nbchannels];
            for (uint i = 0; i < nbchannels; i++) {
                ps_buf[i] = _TIFFmalloc(stripsize);
                lineSizes[i] = scanLineSize / lineSizeCoeffs[i];
            }
            tiffstream = new KisBufferStreamSeperate((uint8**) ps_buf, nbchannels, depth, lineSizes);
            delete [] lineSizes;
        }

        dbgFile << "Scanline size =" << TIFFRasterScanlineSize(image) << " / strip size =" << TIFFStripSize(image) << " / rowsPerStrip =" << rowsPerStrip << " stripsize/rowsPerStrip =" << stripsize / rowsPerStrip;
        uint32 y = 0;
        dbgFile << " NbOfStrips =" << TIFFNumberOfStrips(image) << " rowsPerStrip =" << rowsPerStrip << " stripsize =" << stripsize;
        for (uint32 strip = 0; y < height; strip++) {
            if (planarconfig == PLANARCONFIG_CONTIG) {
                TIFFReadEncodedStrip(image, TIFFComputeStrip(image, y, 0) , buf, (tsize_t) - 1);
            }
            else {
                for (uint i = 0; i < nbchannels; i++) {
                    TIFFReadEncodedStrip(image, TIFFComputeStrip(image, y, i), ps_buf[i], (tsize_t) - 1);
                }
            }
            for (uint32 yinstrip = 0 ; yinstrip < rowsPerStrip && y < height ;) {
                uint linesread = tiffReader->copyDataToChannels(0, y, width, tiffstream);
                y += linesread;
                yinstrip += linesread;
                tiffstream->moveToLine(yinstrip);
            }
            tiffstream->restart();
        }
    }
    tiffReader->finalize();
    delete[] lineSizeCoeffs;
    delete tiffReader;
    delete tiffstream;
    if (planarconfig == PLANARCONFIG_CONTIG) {
        _TIFFfree(buf);
    } else {
        for (uint i = 0; i < nbchannels; i++) {
            _TIFFfree(ps_buf[i]);
        }
        delete[] ps_buf;
    }

    m_image->addNode(KisNodeSP(layer), m_image->rootLayer().data());
    return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisTIFFConverter::buildImage(const QString &filename)
{
    return decode(filename);
}


KisImageSP KisTIFFConverter::image()
{
    return m_image;
}


KisImageBuilder_Result KisTIFFConverter::buildFile(const QString &filename, KisImageSP kisimage, KisTIFFOptions options)
{
    dbgFile << "Start writing TIFF File";
    if (!kisimage)
        return KisImageBuilder_RESULT_EMPTY;

    // Open file for writing
    TIFF *image;
    if ((image = TIFFOpen(QFile::encodeName(filename), "w")) == 0) {
        dbgFile << "Could not open the file for writing" << filename;
        TIFFClose(image);
        return (KisImageBuilder_RESULT_FAILURE);
    }

    // Set the document information
    KoDocumentInfo * info = m_doc->documentInfo();
    QString title = info->aboutInfo("title");
    if (!title.isEmpty()) {
        TIFFSetField(image, TIFFTAG_DOCUMENTNAME, title.toLatin1().constData());
    }
    QString abstract = info->aboutInfo("description");
    if (!abstract.isEmpty()) {
        TIFFSetField(image, TIFFTAG_IMAGEDESCRIPTION, abstract.toLatin1().constData());
    }
    QString author = info->authorInfo("creator");
    if (!author.isEmpty()) {
        TIFFSetField(image, TIFFTAG_ARTIST, author.toLatin1().constData());
    }

    dbgFile << "xres: " << INCH_TO_POINT(kisimage->xRes()) << " yres: " << INCH_TO_POINT(kisimage->yRes());
    TIFFSetField(image, TIFFTAG_XRESOLUTION, INCH_TO_POINT(kisimage->xRes())); // It is the "invert" macro because we convert from pointer-per-inchs to points
    TIFFSetField(image, TIFFTAG_YRESOLUTION, INCH_TO_POINT(kisimage->yRes()));

    KisGroupLayer* root = dynamic_cast<KisGroupLayer*>(kisimage->rootLayer().data());
    if (root == 0) {
        TIFFClose(image);
        return KisImageBuilder_RESULT_FAILURE;
    }

    KisTIFFWriterVisitor* visitor = new KisTIFFWriterVisitor(image, &options);
    if (!visitor->visit(root)) {
        TIFFClose(image);
        return KisImageBuilder_RESULT_FAILURE;
    }

    TIFFClose(image);
    return KisImageBuilder_RESULT_OK;
}


void KisTIFFConverter::cancel()
{
    m_stop = true;
}
