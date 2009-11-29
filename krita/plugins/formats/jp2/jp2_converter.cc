/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_converter.h"

#include <openjpeg.h>

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <QFileInfo>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <kis_iterator.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KMessageBox>

jp2Converter::jp2Converter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

jp2Converter::~jp2Converter()
{
}

/**
 * sample error callback expecting a FILE* client object
 * */
void
error_callback(const char *msg, void *client_data)
{
    FILE *stream = (FILE *) client_data;
    fprintf(stream, "[ERROR] %s", msg);
}

/**
 * sample warning callback expecting a FILE* client object
 * */
void
warning_callback(const char *msg, void *client_data)
{
    FILE *stream = (FILE *) client_data;
    fprintf(stream, "[WARNING] %s", msg);
}

/**
 * sample debug callback expecting no client object
 * */
void
info_callback(const char *msg, void *client_data)
{
    fprintf(stdout, "[INFO] %s", msg);
}

KisImageBuilder_Result jp2Converter::decode(const KUrl& uri)
{
    // decompression parameters
    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);
    // Determine the type
    parameters.decod_format = getFileFormat(uri); // TODO isn't there some magic code ?
    // open the file
    QFile fp(uri.toLocalFile());
    fp.open(QIODevice::ReadOnly);
    QByteArray src = fp.readAll();
    fp.close();
    // Decode the file
    opj_dinfo_t *dinfo = 0;

    bool hasColorSpaceInfo = false;
    /* get a decoder handle */
    switch (parameters.decod_format) {
    case J2K_CFMT: {
        dinfo = opj_create_decompress(CODEC_J2K);
        break;
    }
    case JP2_CFMT: {
        dinfo = opj_create_decompress(CODEC_JP2);
        hasColorSpaceInfo = true;
        break;
    }
    case JPT_CFMT: {
        dinfo = opj_create_decompress(CODEC_JPT);
        break;
    }
    }
    Q_ASSERT(dinfo);

    /* setup the decoder decoding parameters using user parameters */
    opj_setup_decoder(dinfo, &parameters);

    /* open a byte stream */
    opj_cio_t *cio = opj_cio_open((opj_common_ptr) dinfo, (unsigned char*)src.data(), src.length());

    // Setup an event manager
    opj_event_mgr_t event_mgr;    /* event manager */
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
    event_mgr.error_handler = error_callback;
    event_mgr.warning_handler = warning_callback;
    event_mgr.info_handler = info_callback;

    /* catch events using our callbacks and give a local context */
    opj_set_event_mgr((opj_common_ptr) dinfo, &event_mgr, stderr);

    /* decode the stream and fill the image structure */
    opj_image_t *image = opj_decode(dinfo, cio);

    /* close the byte stream */
    opj_cio_close(cio);
    if (!image) {
        opj_destroy_decompress(dinfo);
        return KisImageBuilder_RESULT_FAILURE;
    }

    // Look for the colorspace
    int components = image->numcomps;
    if (image->numcomps == 0) {
        opj_destroy_decompress(dinfo);
        return KisImageBuilder_RESULT_FAILURE;
    }
    int bitdepth = image->comps[0].bpp;
    for (int i = 1; i < components; ++i) {
        if (image->comps[i].bpp != bitdepth) {
            opj_destroy_decompress(dinfo);
            return KisImageBuilder_RESULT_FAILURE;
        }
    }
    dbgFile << "Image has " << components << " components and a bit depth of " << bitdepth << " for color space " << image->color_space;
    if (bitdepth == 0) {
        bitdepth = 8;
    }
    const KoColorSpace* colorSpace = 0;
    QVector<int> channelorder(components);
    if (!hasColorSpaceInfo) {
        if (components == 3) {
            image->color_space = CLRSPC_SRGB;
        } else if (components == 3) {
            image->color_space = CLRSPC_GRAY;
        }
    }
    switch (image->color_space) {
    case CLRSPC_UNKNOWN:
        break;
    case CLRSPC_SRGB: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "");
        }
        if (components != 3) {
            opj_destroy_decompress(dinfo);
            return KisImageBuilder_RESULT_FAILURE;
        }
        channelorder[0] = KoRgbU16Traits::red_pos;
        channelorder[1] = KoRgbU16Traits::green_pos;
        channelorder[2] = KoRgbU16Traits::blue_pos;
        break;
    }
    case CLRSPC_GRAY: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", "");
        }
        if (components != 1) {
            opj_destroy_decompress(dinfo);
            return KisImageBuilder_RESULT_FAILURE;
        }
        channelorder[0] = 0;
        break;
    }
    case CLRSPC_SYCC: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("YUVA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("YUVA8", "");
        }
        if (components != 3) {
            opj_destroy_decompress(dinfo);
            return KisImageBuilder_RESULT_FAILURE;
        }
        channelorder[0] = 0;
        channelorder[1] = 1;
        channelorder[2] = 2;
        break;
    }
    }
    if (!colorSpace) {
        dbgFile << "No colors space found for that image";
        return KisImageBuilder_RESULT_FAILURE;
    }

    // Create the image
    if (m_image == 0) {
        m_image = new KisImage(m_adapter, image->x1, image->y1, colorSpace, "built image");
        m_image->lock();
    }

    // Create the layer
    KisPaintLayerSP layer = new KisPaintLayer(m_image.data(), m_image->nextLayerName(), OPACITY_OPAQUE);
    KisTransaction("", layer->paintDevice());
    m_image->addNode(layer.data(), m_image->rootLayer().data());

    // Set the data
    int pos = 0;
    for (int v = 0; v < image->y1; ++v) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, v, image->x1);
        if (bitdepth == 16) {
            while (!it.isDone()) {
                quint16* px = reinterpret_cast<quint16*>(it.rawData());
                for (int i = 0; i < components; ++i) {
                    px[channelorder[i]] = image->comps[i].data[pos];
                }
                colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                ++pos;
                ++it;
            }
        } else if (bitdepth == 8) {
            while (!it.isDone()) {
                quint8* px = reinterpret_cast<quint8*>(it.rawData());
                for (int i = 0; i < components; ++i) {
                    px[channelorder[i]] = image->comps[i].data[pos];
                }
                colorSpace->setAlpha(px, OPACITY_OPAQUE, 1);
                ++pos;
                ++it;
            }
        }
    }

    m_image->unlock();
    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result jp2Converter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, QApplication::activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP jp2Converter::getImage()
{
    return m_image;
}


KisImageBuilder_Result jp2Converter::buildFile(const KUrl& uri, KisPaintLayerSP layer, const JP2ConvertOptions& options)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP kisimage = layer->image();
    if (!kisimage)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    // Init parameters
    opj_cparameters_t parameters;
    opj_set_default_encoder_parameters(&parameters);
    parameters.cp_comment = "Created by Krita";
    parameters.subsampling_dx = 1;
    parameters.subsampling_dy = 1;
    parameters.cp_disto_alloc = 1;
    parameters.tcp_numlayers = 1;
    parameters.numresolution = options.numberresolution;
    dbgFile << 100 - options.rate;
    parameters.tcp_rates[0] = options.rate;
    

    // Set the colorspace information
    OPJ_COLOR_SPACE clrspc;
    int components;
    QVector<int> channelorder;
    if (layer->colorSpace()->colorModelId() == GrayAColorModelID || layer->colorSpace()->colorModelId() == GrayColorModelID) {
        clrspc = CLRSPC_GRAY;
        components = 1;
        channelorder.resize(components);
        channelorder[0] = 0;
    } else if (layer->colorSpace()->colorModelId() == RGBAColorModelID) {
        clrspc = CLRSPC_SRGB;
        components = 3;
        channelorder.resize(components);
        channelorder[0] = KoRgbU16Traits::red_pos;
        channelorder[1] = KoRgbU16Traits::green_pos;
        channelorder[2] = KoRgbU16Traits::blue_pos;
    } else {
        KMessageBox::error(0, i18n("Cannot export images in %1.\n", layer->colorSpace()->name())) ;
        return KisImageBuilder_RESULT_FAILURE;
    }

    int bitdepth;
    if (layer->colorSpace()->colorDepthId() == Integer8BitsColorDepthID) {
        bitdepth = 8;
//      TODO OpenJpeg does not really support 16bits yet
//     } else if (layer->colorSpace()->colorDepthId() == Integer16BitsColorDepthID) {
//         bitdepth = 16;
    } else {
        KMessageBox::error(0, i18n("Cannot export images in %1.\n", layer->colorSpace()->name())) ;
        return KisImageBuilder_RESULT_FAILURE;
    }

    // Init the image
    opj_image_cmptparm_t image_info[3];

    int width = kisimage->width();
    int height = kisimage->height();
    for (int k = 0; k < components; k++) {
        image_info[k].dx = 1;
        image_info[k].dy = 1;
        image_info[k].w = width;
        image_info[k].h = height;
        image_info[k].x0 = 0;
        image_info[k].y0 = 0;
        image_info[k].prec = 8;
        image_info[k].bpp = bitdepth;
        image_info[k].sgnd = 0;
    }
    opj_image_t *image = opj_image_create(components, image_info, clrspc);
    image->x1 = width;
    image->y1 = height;
    image->x0 = 0;
    image->y0 = 0;

    // Copy the data in the image
    int pos = 0;
    for (int v = 0; v < height; ++v) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, v, image->x1);
        if (bitdepth == 16) {
            while (!it.isDone()) {
                quint16* px = reinterpret_cast<quint16*>(it.rawData());
                for (int i = 0; i < components; ++i) {
                    image->comps[i].data[pos] = px[channelorder[i]];
                }
                ++pos;
                ++it;
            }
        } else if (bitdepth == 8) {
            while (!it.isDone()) {
                quint8* px = reinterpret_cast<quint8*>(it.rawData());
                for (int i = 0; i < components; ++i) {
                    image->comps[i].data[pos] = px[channelorder[i]];
                }
                ++pos;
                ++it;
            }
        }
    }
    
    // coding format
    parameters.decod_format = getFileFormat(uri); // TODO isn't there some magic code ?

    // Decode the file
    opj_cinfo_t *cinfo = 0;

    bool hasColorSpaceInfo = false;
    /* get a decoder handle */
    switch (parameters.decod_format) {
    case J2K_CFMT: {
        cinfo = opj_create_compress(CODEC_J2K);
        break;
    }
    case JP2_CFMT: {
        cinfo = opj_create_compress(CODEC_JP2);
        hasColorSpaceInfo = true;
        break;
    }
    case JPT_CFMT: {
        cinfo = opj_create_compress(CODEC_JPT);
        break;
    }
    }

    // Setup an event manager
    opj_event_mgr_t event_mgr;    /* event manager */
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
    event_mgr.error_handler = error_callback;
    event_mgr.warning_handler = warning_callback;
    event_mgr.info_handler = info_callback;

    /* catch events using our callbacks and give a local context */
    opj_set_event_mgr((opj_common_ptr) cinfo, &event_mgr, stderr);


    /* setup the encoder parameters using the current image and using user parameters */
    opj_setup_encoder(cinfo, &parameters, image);

    opj_cio_t* cio = opj_cio_open((opj_common_ptr) cinfo, 0, 0);

    /* encode the image */
    if (!opj_encode(cinfo, cio, image, parameters.index)) {
        opj_cio_close(cio);
        opj_destroy_compress(cinfo);
        return KisImageBuilder_RESULT_FAILURE;
    }

    // Write to the file
    QFile fp(uri.path());
    fp.open(QIODevice::WriteOnly);
    int length = cio_tell(cio);
    dbgFile << "Length of the file to save: " << length;
    fp.write((char*)cio->buffer, length);
    fp.close();

    opj_cio_close(cio);
    opj_destroy_compress(cinfo);

    return KisImageBuilder_RESULT_OK;
}


void jp2Converter::cancel()
{
    m_stop = true;
}

int jp2Converter::getFileFormat(const KUrl& uri) const
{
    QString extension = QFileInfo(uri.fileName()).suffix().toLower();
    if (extension == "j2k" || extension == "j2c") {
        return J2K_CFMT;
    } else if (extension == "jp2") {
        return JP2_CFMT;
    } else if (extension == "jpt") {
        return JPT_CFMT;
    }
    qFatal("Unsupported extension");
    return -1;
}

#include "jp2_converter.moc"

