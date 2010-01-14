/*
 *  Copyright (c) 2009-2010 Boudewijn Rempt <boud@valdyas.org>
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
/*
 * Inspired by qt-gif-plugin  (http://gitorious.org/qt-gif-plugin)
 * Copyright (C) 2009 Shawn T. Rutledge (shawn.t.rutledge@gmail.com)
 */

#include "gif_converter.h"

#include <QBitArray>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoDocumentInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include <kis_types.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>
#include <kis_random_accessor.h>
#include <kis_paint_device.h>
#include <kis_group_layer.h>
#include "kis_gif_writer_visitor.h"

static const int InterlacedOffset[] = {0, 4, 2, 1};
static const int InterlacedJumps[] = {8, 8, 4, 3};

int doInput(GifFileType* gif, GifByteType* data, int i)
{
    QIODevice* in = (QIODevice*)gif->UserData;
    return in->read((char*)data, i);
}

GifConverter::GifConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
    : m_doc(doc)
    , m_img(0)
{
    Q_UNUSED(adapter);
}

GifConverter::~GifConverter()
{
}

KisNodeSP GifConverter::getNode(GifFileType* gifFile, KisImageWSP kisImage) {

    if (DGifGetImageDesc(gifFile) == GIF_ERROR) {
        warnFile << "Could not read gif image from file";
        return 0;
    }

    KisPaintLayerSP layer = new KisPaintLayer(kisImage, kisImage->nextLayerName(), OPACITY_OPAQUE);

    GifImageDesc image = gifFile->Image;

    layer->setX(image.Left);
    layer->setY(image.Top);

    dbgFile << "Creating layer w" << image.Width << "h" << image.Height << "left" << image.Left << "top" << image.Top;

    Q_ASSERT(gifFile->SBackGroundColor < gifFile->SColorMap->ColorCount);
    GifColorType color = gifFile->SColorMap->Colors[gifFile->SBackGroundColor];
    quint8 fillPixel[4];
    fillPixel[0] = color.Blue;
    fillPixel[1] = color.Green;
    fillPixel[2] = color.Red;
    if (gifFile->SBackGroundColor == m_transparentColorIndex) {
        fillPixel[3] = OPACITY_TRANSPARENT;
    }
    else {
        fillPixel[3] = OPACITY_OPAQUE;
    }
    layer->paintDevice()->fill(image.Left, image.Top, image.Width, image.Height,
                               fillPixel);

    GifPixelType* line = new GifPixelType[image.Width];
    KisRandomAccessorPixel accessor = layer->paintDevice()->createRandomAccessor(image.Left, image.Top);
    if (image.Interlace) {
        for (int i = 0; i < 4; i++) {
            for (int row = image.Top + InterlacedOffset[i]; row < image.Top + image.Height; row += InterlacedJumps[i]) {
                if (DGifGetLine(gifFile, line, image.Width) == GIF_ERROR) {
                    return 0;
                }
                for (int col = 0; col < image.Width; ++col) {

                    accessor.moveTo(col + image.Left, row);

                    GifPixelType colorIndex = line[col];
                    color = image.ColorMap->Colors[colorIndex];

                    quint8* dst = accessor.rawData();
                    KoRgbTraits<quint8>::setRed(dst, color.Red);
                    KoRgbTraits<quint8>::setGreen(dst, color.Blue);
                    KoRgbTraits<quint8>::setBlue(dst, color.Red);
                    if (colorIndex == m_transparentColorIndex) {
                        layer->colorSpace()->setAlpha(dst, OPACITY_TRANSPARENT, 1);
                    }
                    else {
                        layer->colorSpace()->setAlpha(dst, OPACITY_OPAQUE, 1);
                    }
                }
            }
        }
    } else {
        for (int row = image.Top; row < image.Top + image.Height; ++ row) {
            if (DGifGetLine(gifFile, line, image.Width) == GIF_ERROR) {
                return 0;
            }
            for (int col = 0; col < image.Width; ++col) {

                accessor.moveTo(col + image.Left, row);

                GifPixelType colorIndex = line[col];

                if (image.ColorMap && colorIndex < image.ColorMap->ColorCount) {
                    color = image.ColorMap->Colors[colorIndex];
                }
                else if (gifFile->SColorMap && colorIndex < gifFile->SColorMap->ColorCount) {
                    color = gifFile->SColorMap->Colors[colorIndex];
                }
                else {
                    color.Red = 0;
                    color.Green = 0;
                    color.Blue = 0;
                }

                quint8* dst = accessor.rawData();
                KoRgbTraits<quint8>::setRed(dst, color.Red);
                KoRgbTraits<quint8>::setGreen(dst, color.Blue);
                KoRgbTraits<quint8>::setBlue(dst, color.Red);
                if (colorIndex == m_transparentColorIndex) {
                    layer->colorSpace()->setAlpha(dst, OPACITY_TRANSPARENT, 1);
                }
                else {
                    layer->colorSpace()->setAlpha(dst, OPACITY_OPAQUE, 1);
                }
            }
        }
    }

    delete[] line;

    dbgFile << "created node" << layer << layer->name();
    return layer.data();
}

KisImageBuilder_Result GifConverter::decode(const KUrl& uri)
{
    // open the file

    QFile f(uri.toLocalFile());
    if (!f.exists()) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    GifFileType* gifFile = DGifOpen(&f, doInput);
    if (!gifFile) {
        warnFile << "Could not read gif file" << uri;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "reading gif file" << uri;

    // Creating the KisImageWSP
    KisImageWSP img = new KisImage(m_doc->undoAdapter(),
                                   gifFile->SWidth,
                                   gifFile->SHeight,
                                   KoColorSpaceRegistry::instance()->rgb8(),
                                   uri.toLocalFile());
    Q_CHECK_PTR(img);
    img->lock();

    dbgFile << "image" << img << "width" << gifFile->SWidth << "height" << gifFile->SHeight;

    GifRecordType recordType = UNDEFINED_RECORD_TYPE;

    while (recordType != TERMINATE_RECORD_TYPE){
        DGifGetRecordType(gifFile, &recordType);
        switch (recordType) {
        case IMAGE_DESC_RECORD_TYPE:
            {
                dbgFile << "reading IMAGE_DESC_RECORD_TYPE";
                KisNodeSP node = getNode(gifFile, img);
                if (!node) {
                    return KisImageBuilder_RESULT_FAILURE;
                }
                img->addNode(node);
            }
            break;
        case EXTENSION_RECORD_TYPE:
            {
                dbgFile << "reading EXTENSION_RECORD_TYPE";
                int extCode, len;
                GifByteType* extData = 0;
                if (DGifGetExtension(gifFile, &extCode, &extData) == GIF_ERROR) {
                    warnFile << "Error reading extension";
                    return KisImageBuilder_RESULT_FAILURE;
                }

                if (extData != 0) {
                    len = extData[0];

                    dbgFile << "\tCode" << extCode << "lenght" << len;

                    switch(extCode) {
                    case GRAPHICS_EXT_FUNC_CODE:
                        {
                            dbgFile << "\t GRAPHICS_EXT_FUNC_CODE";
                            // There are lots of fields, but only one that interests us, since we
                            // wouldn't store and save the animation information anyway.
                            if (extData[1] & 0x01)
                            {
                                m_transparentColorIndex = extData[3];
                                dbgFile << "Transparent color index" << m_transparentColorIndex;
                            }
                        }
                        break;
                    case COMMENT_EXT_FUNC_CODE:
                        {
                            dbgFile << "COMMENT_EXT_FUNC_CODE";
                            QByteArray comment((char*)(extData + 1), len);
                            m_doc->documentInfo()->setAboutInfo("comments", comment);
                        }
                        break;
                    case PLAINTEXT_EXT_FUNC_CODE:
                        dbgFile << "PLAINTEXT_EXT_FUNC_CODE";
                        // XXX: insert a monospace text shape layer? Nobody used this extension anyway...
                        break;
                    case APPLICATION_EXT_FUNC_CODE:
                        dbgFile << "APPLICATION_EXT_FUNC_CODE";
                        break;
                    }
                }
            }
            break;
        case TERMINATE_RECORD_TYPE:
            dbgFile << "Reading TERMINATE_RECORD_TYPE";
            break;
        case UNDEFINED_RECORD_TYPE:
        default:
            {
                warnFile << "Found an undefined record type:" << recordType;
            }

        }
    }

    img->unlock();
    m_img = img;
    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result GifConverter::buildImage(const KUrl& uri)
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


KisImageWSP GifConverter::image()
{
    return m_img;
}


KisImageBuilder_Result GifConverter::buildFile(const KUrl& uri, KisImageWSP image)
{
    if (!image)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    m_img = image;

    // get a list of all layers converted to 8 bit indexed QImages
    KisGifWriterVisitor visitor;
    m_img->rootLayer()->accept(visitor);


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


void GifConverter::cancel()
{
    m_stop = true;
}

#include "gif_converter.moc"

