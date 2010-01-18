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

#include <kis_fill_painter.h>
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
    : m_transparentColorIndex(-1)
    , m_doc(doc)
    , m_img(0)
{
    Q_UNUSED(adapter);
}

GifConverter::~GifConverter()
{
}

bool GifConverter::convertLine(GifFileType* gifFile, GifPixelType* line, int row, GifImageDesc &image, KisRandomAccessorPixel &accessor, KisPaintLayerSP layer) {

    GifColorType color;

    if (DGifGetLine(gifFile, line, image.Width) == GIF_ERROR) {
        return false;
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
            dbgFile << "color" << colorIndex << "not in any map";
            color.Red = 255;
            color.Green = 0;
            color.Blue = 0;
        }

        quint8* dst = accessor.rawData();
        KoRgbTraits<quint8>::setRed(dst, color.Red);
        KoRgbTraits<quint8>::setGreen(dst, color.Green);
        KoRgbTraits<quint8>::setBlue(dst, color.Blue);

        if (colorIndex == m_transparentColorIndex) {
            layer->colorSpace()->setAlpha(dst, OPACITY_TRANSPARENT, 1);
        }
        else {
            layer->colorSpace()->setAlpha(dst, OPACITY_OPAQUE, 1);
        }
    }
    return true;
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

    // always prefil the layer with transparency
    KoRgbTraits<quint8>::setRed(fillPixel, color.Red);
    KoRgbTraits<quint8>::setGreen(fillPixel, color.Green);
    KoRgbTraits<quint8>::setBlue(fillPixel, color.Blue);
    layer->colorSpace()->setAlpha(fillPixel, OPACITY_TRANSPARENT, 1);

    layer->paintDevice()->fill(0, 0, m_img->width(), m_img->height(), fillPixel);

    GifPixelType* line = new GifPixelType[image.Width];
    KisRandomAccessorPixel accessor = layer->paintDevice()->createRandomAccessor(image.Left, image.Top);
    if (image.Interlace) {
        for (int i = 0; i < 4; i++) {
            for (int row = image.Top + InterlacedOffset[i]; row < image.Top + image.Height; row += InterlacedJumps[i]) {
                convertLine(gifFile, line, row, image, accessor, layer);
            }
        }
    } else {
        for (int row = image.Top; row < image.Top + image.Height; ++ row) {
            convertLine(gifFile, line, row, image, accessor, layer);
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
    m_img = new KisImage(m_doc->undoAdapter(),
                                   gifFile->SWidth,
                                   gifFile->SHeight,
                                   KoColorSpaceRegistry::instance()->rgb8(),
                                   uri.toLocalFile());
    Q_CHECK_PTR(m_img);
    m_img->lock();

    dbgFile << "image" << m_img << "width" << gifFile->SWidth << "height" << gifFile->SHeight;

    GifRecordType recordType = UNDEFINED_RECORD_TYPE;

    while (recordType != TERMINATE_RECORD_TYPE){
        DGifGetRecordType(gifFile, &recordType);
        switch (recordType) {
        case IMAGE_DESC_RECORD_TYPE:
            {
                dbgFile << "reading IMAGE_DESC_RECORD_TYPE";
                KisNodeSP node = getNode(gifFile, m_img);
                if (!node) {
                    return KisImageBuilder_RESULT_FAILURE;
                }
                m_img->addNode(node);
            }
            break;
        case EXTENSION_RECORD_TYPE:
            {
                dbgFile << "reading EXTENSION_RECORD_TYPE";
                int extCode;
                GifByteType* extData = 0;
                if (DGifGetExtension(gifFile, &extCode, &extData) == GIF_ERROR) {
                    warnFile << "Error reading extension";
                    return KisImageBuilder_RESULT_FAILURE;
                }

                if (extData != 0) {
                    int len = extData[0];

                    dbgFile << "\tCode" << extCode << "length" << len << extData[0] << extData[1] << extData[2] << extData[3] << extData[4];

                    switch(extCode) {
                    case GRAPHICS_EXT_FUNC_CODE:
                        {
                            dbgFile << "\t GRAPHICS_EXT_FUNC_CODE";
                            // There are lots of fields, but only one that interests us, since we
                            // wouldn't store and save the animation information anyway.
                            if (extData[1] & 0x01)
                            {
                                m_transparentColorIndex = extData[4];
                                dbgFile << "\t\tTransparent color index" << m_transparentColorIndex;
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

    m_img->unlock();
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


int doOutput(GifFileType* gif, const GifByteType * data, int i)
{
    QIODevice* out = (QIODevice*)gif->UserData;
    return out->write((const char*)data, i);
}

int fillColorMap(const QImage& image, ColorMapObject& cmap) {

    Q_ASSERT(image.format() == QImage::Format_Indexed8);

    QVector<QRgb> colorTable = image.colorTable();

    dbgFile << "Color table size" << colorTable.size();

    Q_ASSERT(colorTable.size() <= 256);

    // numColors must be a power of 2
    int numColors = 1 << BitSize(image.numColors());

    cmap.ColorCount = numColors;
    cmap.BitsPerPixel = 8;
    GifColorType* colorValues = (GifColorType*)malloc(cmap.ColorCount * sizeof(GifColorType));
    cmap.Colors = colorValues;
    int c = 0;
    for(; c < image.numColors(); ++c)
    {
        colorValues[c].Red = qRed(colorTable[c]);
        colorValues[c].Green = qGreen(colorTable[c]);
        colorValues[c].Blue = qBlue(colorTable[c]);
    }
    // In case we had an actual number of colors that's not a power of 2,
    // fill the rest with something (black perhaps).
    for (; c < numColors; ++c)
    {
        colorValues[c].Red = 0;
        colorValues[c].Green = 0;
        colorValues[c].Blue = 0;
    }

    return numColors;
}


KisImageBuilder_Result GifConverter::buildFile(const KUrl& uri, KisImageWSP image)
{
    if (!image)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    // Open file for writing
    QFile file(QFile::encodeName(uri.toLocalFile()));
    if (!file.open(QIODevice::WriteOnly)) {
        return (KisImageBuilder_RESULT_FAILURE);
    }

    m_img = image;

    // get a list of all layers converted to 8 bit indexed QImages
    KisGifWriterVisitor visitor;
    m_img->rootLayer()->accept(visitor);

    dbgFile << "Created" << visitor.m_layers.count() << "indexed layers";

    // get a global colormap from the projection
    QImage projection = m_img->projection()->convertToQImage(0).convertToFormat(QImage::Format_Indexed8);
    ColorMapObject cmap;
    int numColors = fillColorMap(projection, cmap);

    dbgFile << "Filled colormap with" << numColors << "colors";

    EGifSetGifVersion("89a");
    GifFileType* gif = EGifOpen(&file, doOutput);

    if (EGifPutScreenDesc(gif, m_img->width(), m_img->height(), numColors, 0, &cmap) == GIF_ERROR) {
        dbgFile << "Failed to put the gif screen" << GifLastError();
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "gif screen width" << m_img->width() << ", height" << m_img->height();

    QString comments = m_doc->documentInfo()->aboutInfo("comments");
    if (!comments.isEmpty()) {
        dbgFile << "Comments:" << comments;
        EGifPutComment(gif, comments.toAscii().constData());
    }

    // disposition for the frames go in packed fields here, but we only
    // care that Qt puts the transparent color in the first entry in its
    // colortable.
    char extensionBlock[4];
    extensionBlock[0] = 9;
    extensionBlock[1] = 20;
    extensionBlock[2] = 0;
    extensionBlock[3] = 0;

    EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, 4, &extensionBlock);


    // now save all the layers as gif images.
    foreach(const IndexedLayer layer, visitor.m_layers) {

        ColorMapObject cmapLayer;
        int numColorsLayer = fillColorMap(layer.image, cmapLayer);
        Q_ASSERT(numColorsLayer <= 256);

        QRect rc(layer.topLeft, layer.image.size());
        // Make sure the individual layers are not outside the gif screen bounds
        rc = m_img->bounds().intersected(rc);

        dbgFile << "layer size" << rc << "image bounds" << m_img->bounds();

        if (EGifPutImageDesc(gif, rc.x(), rc.y(), rc.width(), rc.height(), 0, &cmapLayer) == GIF_ERROR) {
            dbgFile << "Failed to add layer" << GifLastError();
            return KisImageBuilder_RESULT_FAILURE;
        }

        int rowcount = rc.height();
        int lineLength = rc.width();

        dbgFile << "rows" << rowcount << "line length" << lineLength;

        for (int row = 0; row < rowcount; ++row)
        {
            const uchar* line = layer.image.scanLine(row);
            if (EGifPutLine(gif, (GifPixelType*)line, lineLength) == GIF_ERROR) {
                dbgFile << "Failed to save scanline" << GifLastError() << "at row" << row;
                return KisImageBuilder_RESULT_FAILURE;
            }
        }

    }

    EGifCloseFile(gif);

    return KisImageBuilder_RESULT_OK;
}


void GifConverter::cancel()
{
    m_stop = true;
}

#include "gif_converter.moc"

