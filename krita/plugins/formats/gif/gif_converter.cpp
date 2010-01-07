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

#include <QBitArray>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

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

#define CM_RED           0
#define CM_GREEN         1
#define CM_BLUE          2

#define MAX_LZW_BITS     12

#define INTERLACE          0x40
#define LOCALCOLORMAP      0x80
#define BitSet(byte, bit)  (((byte) & (bit)) == (bit))

typedef quint8 CMap[3][MAXCOLORMAPSIZE];

struct GifScreen
{
    quint16 width;
    quint16 height;
    quint32 bitPixel;
    quint32 colorResolution;
    quint8 background;
    quint8 aspectRatio;
    const KoColorSpace* colorSpace;
    CMap colorMap;
};

static struct
{
    qint32 transparent;
    qint32 delayTime;
    qint32 inputFlag;
    qint32 disposal;
} Gif89 = { -1, -1, -1, 0 };

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

bool readColorMap(QDataStream& in, int colorResolution, CMap& colorMap, bool* grayscaleFlag)
{
    *grayscaleFlag = true;

    for(quint32 i = 0; i < colorResolution; ++i) {
        for (quint32 j = 0; j < 3; ++j) {
            if (!in.atEnd()) {
                in >> colorMap[j][i];
            }
            else {
                warnFile << "Could not read colormap";
                return false;
            }
        }
        *grayscaleFlag &= (colorMap[i][0] == colorMap[i][1] &&
                          colorMap[i][1] == colorMap[i][2]);
    }
    return true;
}


KisImageBuilder_Result gifConverter::decode(const KUrl& uri)
{
    // open the file

    QFile f(uri.toLocalFile());
    if (!f.exists()) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    // gifs are small, read the whole thing in one go
    //QByteArray ba = f.readAll();
    QDataStream in(&f);
    in.setByteOrder(QDataStream::LittleEndian);

    char headerbytes[6];
    if (!in.readRawData(headerbytes, sizeof(headerbytes)) == sizeof(headerbytes)) {
        return KisImageBuilder_RESULT_UNSUPPORTED;
    }

    QString header(headerbytes);
    if (!(    qstrcmp(headerbytes, "GIF")
          && (qstrcmp(headerbytes + 3, "87a") || qstrcmp(headerbytes + 3,"89a")))) {
        warnFile << "Not a gif file" << header << "," << uri;
        return KisImageBuilder_RESULT_UNSUPPORTED;
    }

    GifScreen gifScreen;
    memset(&gifScreen, 0, sizeof(GifScreen));

    in >> gifScreen.width;
    in >> gifScreen.height;

    /*
     <Packed Fields>  =      Global Color Table Flag       1 Bit
                             Color Resolution              3 Bits
                             Sort Flag                     1 Bit
                             Size of Global Color Table    3 Bits
    */
    quint8 packedFields;
    in >> packedFields;
    gifScreen.bitPixel = 2 << (packedFields & 0x07);
    gifScreen.colorResolution = (((packedFields & 0x70) >> 3) + 1);
    in >> gifScreen.background;
    in >> gifScreen.aspectRatio;

    if (BitSet(packedFields, LOCALCOLORMAP)) {

        bool grayscaleFlag;

        if (!readColorMap(in, gifScreen.colorResolution, gifScreen.colorMap, &grayscaleFlag)) {
            warnFile << "Could not read colormap";
            return KisImageBuilder_RESULT_FAILURE;
        }
        if (grayscaleFlag) {
            gifScreen.colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", "");
        }
        else {
            gifScreen.colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        }
    }

    qDebug() << "GifScreen" << gifScreen.width << gifScreen.height << gifScreen.bitPixel
             << gifScreen.background << gifScreen.aspectRatio << gifScreen.colorSpace;

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


    KisPaintLayerSP layer = new KisPaintLayer(m_img, m_img->nextLayerName(), OPACITY_OPAQUE);
    m_img->addNode(layer);
    return KisImageBuilder_RESULT_FAILURE;
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

