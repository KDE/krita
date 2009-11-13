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

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>
#include <QFileInfo>
#include <KoColorSpaceRegistry.h>

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

    /* get a decoder handle */
    switch (parameters.decod_format) {
    case J2K_CFMT: {
        dinfo = opj_create_decompress(CODEC_J2K);
    }
    case JP2_CFMT: {
        dinfo = opj_create_decompress(CODEC_JP2);
    }
    case JPT_CFMT: {
        dinfo = opj_create_decompress(CODEC_JPT);
    }
    }
    Q_ASSERT(dinfo);

    /* setup the decoder decoding parameters using user parameters */
    opj_setup_decoder(dinfo, &parameters);

    /* open a byte stream */
    opj_cio_t *cio = opj_cio_open((opj_common_ptr) dinfo, (unsigned char*)src.data(), src.length());

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

    const KoColorSpace* colorSpace = 0;
    switch (image->color_space) {
    case CLRSPC_UNKNOWN:
        break;
    case CLRSPC_SRGB: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "");
        }
        break;
    }
    case CLRSPC_GRAY: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", "");
        }
        break;
    }
    case CLRSPC_SYCC: {
        if (bitdepth == 16) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("YUVA16", "");
        } else if (bitdepth == 8) {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("YUVA8", "");
        }
        break;
    }
    }
    if (!colorSpace) {
        return KisImageBuilder_RESULT_FAILURE;
    }

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


KisImageWSP jp2Converter::image()
{
    return m_img;
}


KisImageBuilder_Result jp2Converter::buildFile(const KUrl& uri, KisPaintLayerSP layer)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP img = layer -> image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
#if 0
    FILE *fp = fopen(QFile::encodeName(uri.path()), "wb");
    if (!fp) {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    uint height = img->height();
    uint width = img->width();
#endif

    return KisImageBuilder_RESULT_OK;
}


void jp2Converter::cancel()
{
    m_stop = true;
}

int jp2Converter::getFileFormat(const KUrl& uri) const
{
    QString extension = QFileInfo(uri.fileName()).suffix();
    if (extension == "j2k" || extension == "j2c") {
        return J2K_CFMT;
    } else if (extension == "jp2") {
        return JP2_CFMT;
    } else if (extension == "jpt") {
        return JPT_CFMT;
    }
}

#include "jp2_converter.moc"

