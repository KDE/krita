/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "gif_converter.h"

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_types.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>

#define MAXCOLORMAPSIZE  256

#define MAX_LZW_BITS     12

#define INTERLACE          0x40
#define LOCALCOLORMAP      0x80
#define BitSet(byte, bit)  (((byte) & (bit)) == (bit))

typedef quint8 CMap[3][MAXCOLORMAPSIZE];

struct GifScreen
{
    quint8 width;
    quint8 height;
    quint32 bitPixel;
    quint32 colorResolution;
    quint8 background;
    quint8 aspectRatio;
    const KoColorSpace* colorSpace;
    CMap colorMap;
};


gifConverter::gifConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

gifConverter::~gifConverter()
{

}

KisImageBuilder_Result gifConverter::decode(const KUrl& uri)
{
    // open the file

    QFile f(uri.toLocalFile());
    if (f.exists()) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    QDataStream in(&f);
    in.setByteOrder(QDataStream::BigEndian);

    QString magic;
    in >> magic;
    if (magic != "GIF") {
        warnFile << "Not a gif file" << uri;
        return KisImageBuilder_RESULT_FAILURE;
    }

    QString version;
    in >> version;
    if (version != "87a" && version != "89a") {
        warnFile << "Wrong gif version: " << version;
        return KisImageBuilder_RESULT_FAILURE;
    }

    GifScreen gifScreen;
    in >> gifScreen.width;
    in >> gifScreen.height;
    /*
           <Packed Fields>  =      Global Color Table Flag       1 Bit
                             Color Resolution              3 Bits
                             Sort Flag                     1 Bit
                             Size of Global Color Table    3 Bits
    */
    quint8 packedFields;
    gifScreen.bitPixel = 2 << (packedFields & 0x07);
    gifScreen.colorResolution = (((packedFields & 0x70) >> 3) + 1);
    in >> gifScreen.background;
    in >> gifScreen.aspectRatio;

    if (BitSet(packedFields, LOCALCOLORMAP)) {

        bool grayscaleFlag = true;

        for(quint32 i = 0; i < gifScreen.colorResolution; ++i) {
            for (quint32 j = 0; j < 3; ++j) {
                if (!in.atEnd()) {
                    in >> gifScreen.colorMap[j][i];
                }
                else {
                    warnFile << "Could not read colormap";
                    return KisImageBuilder_RESULT_FAILURE;
                }
            }
            grayscaleFlag &= (gifScreen.colorMap[i][0] == gifScreen.colorMap[i][1] &&
                              gifScreen.colorMap[i][1] == gifScreen.colorMap[i][2]);
        }
        if (grayscaleFlag) {
            gifScreen.colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", "");
        }
        else {
            gifScreen.colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        }
    }

    // Creating the KisImageWSP
    if(!m_img) {
        m_img = new KisImage(m_doc->undoAdapter(),
                             gifScreen.width,
                             gifScreen.height,
                             gifScreen.colorSpace,
                             uri.toLocalFile());
        Q_CHECK_PTR(m_img);
        m_img->lock();
    }



    //KisPaintLayerSP layer = new KisPaintLayer(m_img.data(), m_img->nextLayerName(), quint8_MAX));

    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result gifConverter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath( tmpFile );
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP gifConverter::image()
{
    return m_img;
}


KisImageBuilder_Result gifConverter::buildFile(const KUrl& uri, KisPaintLayerSP layer)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP img = layer->image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
#if 0
    FILE *fp = fopen(QFile::encodeName(uri.path()), "wb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    uint height = img->height();
    uint width = img->width();
#endif

    return KisImageBuilder_RESULT_OK;
}


void gifConverter::cancel()
{
    m_stop = true;
}

#include "gif_converter.moc"

