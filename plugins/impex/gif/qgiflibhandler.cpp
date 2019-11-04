/*
 * Copyright (C) 2009 Shawn T. Rutledge (shawn.t.rutledge@gmail.com)
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 */
#include "qgiflibhandler.h"
#include <QDebug>
#include <QVariant>
#include <gif_lib.h>
#include <string.h>		// memset
#include <QPainter>

extern int _GifError;

static const int InterlacedOffset[] = { 0, 4, 2, 1 };	/* The way Interlaced image should */
static const int InterlacedJumps[] = { 8, 8, 4, 2 };	/* be read - offsets and jumps... */

int doOutput(GifFileType* gif, const GifByteType * data, int i)
{
    QIODevice* out = (QIODevice*)gif->UserData;
    //	qDebug("given %d bytes to write; device is writeable? %d", i, out->isWritable());
    return out->write((const char*)data, i);
}

int doInput(GifFileType* gif, GifByteType* data, int i)
{
    QIODevice* in = (QIODevice*)gif->UserData;
    return in->read((char*)data, i);
}

QGIFLibHandler::QGIFLibHandler()
    : QImageIOHandler()
{
}

bool QGIFLibHandler::canRead () const
{
    if (canRead(device())) {
        setFormat("gif");
        return true;
    }
    return false;
}

bool QGIFLibHandler::read ( QImage * image )
{
    // The contents of this function are based on gif2rgb.c, from the giflib source.
//    qDebug("QGIFLibHandler::read into image with size %d x %d", image->size().width(), image->size().height());

    int err;
    GifFileType* gifFile = DGifOpen(device(), doInput, &err);
    if (!gifFile) {
        qWarning() << "Received error code" << err;
        return false;
    }
//    qDebug("dimensions %d x %d", gifFile->SWidth, gifFile->SHeight);

    *image = QImage(gifFile->SWidth, gifFile->SHeight, QImage::Format_Indexed8);

    GifRecordType recordType;
    ColorMapObject* ColorMap;

    int	i, row, imageNum = 0, topRow, width, height;
    int transColor = -1;
    do
    {
        DGifGetRecordType(gifFile, &recordType);
        switch (recordType)
        {
        case IMAGE_DESC_RECORD_TYPE:
            if (DGifGetImageDesc(gifFile) == GIF_ERROR)
            {
                qWarning("QGIFLibHandler::read: error %d", gifFile->Error);
                return false;
            }
            topRow = gifFile->Image.Top; /* Image Position relative to Screen. */
            width = gifFile->Image.Width;
            height = gifFile->Image.Height;
            //qDebug("Image %d at (%d, %d) [%dx%d]", ++imageNum, gifFile->Image.Left, topRow, width, height);
            if (gifFile->Image.Left + width > gifFile->SWidth ||
                    gifFile->Image.Top + height > gifFile->SHeight)
            {
                qWarning("Image %d is not confined to screen dimension, aborted.", imageNum);
                return false;
            }

            // Pre-fill with background color
//            qDebug("background color is at index %d", gifFile->SBackGroundColor);
            image->fill(gifFile->SBackGroundColor);

            // Now read the image data
            if (gifFile->Image.Interlace)
            {
                /* Need to perform 4 passes on the images: */
                for (i = 0; i < 4; i++)
                    for (row = topRow + InterlacedOffset[i]; row < topRow + height;
                         row += InterlacedJumps[i])
                    {
                        if (DGifGetLine(gifFile, image->scanLine(row), width) == GIF_ERROR)
                        {
                            qWarning("QGIFLibHandler::read: error %d", gifFile->Error);
                            return false;
                        }
                        //						else
                        //							qDebug("got row %d: %d %d %d %d %d %d %d %d ...", row,
                        //								image->scanLine(row)[0], image->scanLine(row)[1], image->scanLine(row)[2], image->scanLine(row)[3],
                        //								image->scanLine(row)[4], image->scanLine(row)[5], image->scanLine(row)[6], image->scanLine(row)[7]);
                    }
            }
            else
            {
                for (row = 0; row < height; row++)
                {
                    if (DGifGetLine(gifFile, image->scanLine(row), width) == GIF_ERROR)
                    {
                        qWarning("QGIFLibHandler::read: error %d", gifFile->Error);
                        return false;
                    }
                    //					else
                    //						qDebug("got row %d: %d %d %d %d %d %d %d %d ...", row,
                    //							image->scanLine(row)[0], image->scanLine(row)[1], image->scanLine(row)[2], image->scanLine(row)[3],
                    //							image->scanLine(row)[4], image->scanLine(row)[5], image->scanLine(row)[6], image->scanLine(row)[7]);
                }
            }
            break;
        case EXTENSION_RECORD_TYPE:
        {
            int extCode;
            GifByteType* extData;
            /* Skip any extension blocks in file: */
            if (DGifGetExtension(gifFile, &extCode, &extData) == GIF_ERROR)
            {
                qWarning("QGIFLibHandler::read: error %d", gifFile->Error);
                return false;
            }
            while (extData != NULL)
            {
                int len = extData[0];
                switch (extCode)
                {
                case GRAPHICS_EXT_FUNC_CODE:	// Graphics control extension
//                    qDebug("graphics control: %x %x %x %x %x", extData[0], extData[1], extData[2], extData[3], extData[4]);
                    // Should be block size, packed fields, delay time,
                    // transparent color, block terminator
                    // see doc/gif89.txt in libgif source package
                    // If the trans bit is set in packed fields,
                    // then set the trans color to the one given
                    if (extData[1] & 0x01)
                    {
                        transColor = extData[3];
//                        qDebug("transparent color is at index %d", transColor);
                        /// @todo is it correct to override default fill color?
                        //							image->fill(transColor);
                    }
                    break;
                case COMMENT_EXT_FUNC_CODE:
                {
                    QByteArray comment((char*)(extData + 1), len);
                    //							qDebug("comment of len %d: \"%s\"", len, comment.constData());
                    image->setText("Description", comment);
                }
                    break;
                case PLAINTEXT_EXT_FUNC_CODE:
                    break;
                }
                if (DGifGetExtensionNext(gifFile, &extData) == GIF_ERROR)
                {
                    qWarning("QGIFLibHandler::read: error %d", gifFile->Error);
                    return false;
                }
            }
        }
            break;
        case TERMINATE_RECORD_TYPE:
            break;
        default:
            break;
        }
    }
    while (recordType != TERMINATE_RECORD_TYPE);

    //	BackGround = gifFile->SBackGroundColor;
    ColorMap = (gifFile->Image.ColorMap
                ? gifFile->Image.ColorMap
                : gifFile->SColorMap);
    if (!ColorMap)
    {
        qWarning("QGIFLibHandler::read: Image does not have a colormap");
        return false;
    }
    int ccount = ColorMap->ColorCount;
    image->setColorCount(ccount);
    for (i = 0; i < ccount; ++i)
    {
        GifColorType gifColor = ColorMap->Colors[i];
        QRgb color = gifColor.Blue | (gifColor.Green << 8) | (gifColor.Red << 16);
        // If this is not the transparent color,
        // set the alpha to opaque.
        if (i != transColor)
            color |= 0xff << 24;
        //		qDebug("color %d: 0x%X", i, color);
        image->setColor(i, color);
    }

    return true;
}

bool QGIFLibHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("QGIFLibHandler::canRead() called with no device");
        return false;
    }

    char head[6];
    if (device->peek(head, sizeof(head)) == sizeof(head))
        return qstrncmp(head, "GIF87a", 6) == 0
                || qstrncmp(head, "GIF89a", 6) == 0;
    return false;
}

bool QGIFLibHandler::write ( const QImage & image )
{
    QImage toWrite(image);
    /// @todo how to specify dithering method
    if (toWrite.colorCount() == 0 || toWrite.colorCount() > 256)
        toWrite = image.convertToFormat(QImage::Format_Indexed8);

    QVector<QRgb> colorTable = toWrite.colorTable();
    ColorMapObject cmap;
    // colorCount must be a power of 2
    int colorCount = 1 << GifBitSize(toWrite.colorCount());
    cmap.ColorCount = colorCount;
    cmap.BitsPerPixel = 8;	/// @todo based on colorCount (or not? we did ask for Format_Indexed8, so the data is always 8-bit, right?)
    GifColorType* colorValues = (GifColorType*)malloc(cmap.ColorCount * sizeof(GifColorType));
    cmap.Colors = colorValues;
    int c = 0;
    for(; c < toWrite.colorCount(); ++c)
    {
//        qDebug("color %d has %02X%02X%02X", c, qRed(colorTable[c]), qGreen(colorTable[c]), qBlue(colorTable[c]));
        colorValues[c].Red = qRed(colorTable[c]);
        colorValues[c].Green = qGreen(colorTable[c]);
        colorValues[c].Blue = qBlue(colorTable[c]);
    }
    // In case we had an actual number of colors that's not a power of 2,
    // fill the rest with something (black perhaps).
    for (; c < colorCount; ++c)
    {
        colorValues[c].Red = 0;
        colorValues[c].Green = 0;
        colorValues[c].Blue = 0;
    }
    /// @todo transparent GIFs (use alpha?)


    /// @todo write to m_device
    int err;
    GifFileType *gif = EGifOpen(device(), doOutput, &err);

    /// @todo how to specify which version, or decide based on features in use
    // Because of this call, libgif is not re-entrant
    EGifSetGifVersion(gif, true);

    /// @todo how to specify background
    if (EGifPutScreenDesc(gif, toWrite.width(), toWrite.height(), colorCount, 0, &cmap) == GIF_ERROR) {
        qWarning("EGifPutScreenDesc returned error %d", gif->Error);
    }

    QVariant descText = option(QImageIOHandler::Description);
    if (descText.type() == QVariant::String)
    {
        QString comment = descText.toString();
        // Will be something like "Description: actual text" or just
        // ": actual text", so remove everything leading up to and
        // including the first colon and the space following it.
        int idx = comment.indexOf(": ");
        if (idx >= 0)
            comment.remove(0, idx + 2);
        //		qDebug() << "comment:" << comment;
        if (!comment.isEmpty())
            EGifPutComment(gif, comment.toUtf8().constData());
    }
    //	else
    //		qDebug("description is of qvariant type %d", descText.type());

    /// @todo foreach of multiple images in an animation...
    if (EGifPutImageDesc(gif, 0, 0, toWrite.width(), toWrite.height(), 0, &cmap) == GIF_ERROR)
        qWarning("EGifPutImageDesc returned error %d", gif->Error);

    int lc = toWrite.height();

    // NOTE: we suppose that the pixel size is exactly 1 byte, right now we
    //       cannot save anything else
    int llen = toWrite.width();

    //	qDebug("will write %d lines, %d bytes each", lc, llen);

    for (int l = 0; l < lc; ++l)
    {
        uchar* line = toWrite.scanLine(l);
        if (EGifPutLine(gif, (GifPixelType*)line, llen) == GIF_ERROR)
        {
            int i = gif->Error;
            qWarning("EGifPutLine returned error %d", i);
        }
    }

    EGifCloseFile(gif, &err);
    free(colorValues);

    return true;
}

bool QGIFLibHandler::supportsOption ( ImageOption option ) const
{
    //	qDebug("supportsOption %d", option);
    switch (option)
    {
    // These are relevant only for reading
    case QImageIOHandler::ImageFormat:
    case QImageIOHandler::Size:
        // This is relevant for both reading and writing
    case QImageIOHandler::Description:
        return true;
        break;
    default:
        return false;
    }
}

void QGIFLibHandler::setOption ( ImageOption option, const QVariant & value )
{
    //	qDebug("setOption given option %d, variant of type %d", option, value.type());
    if (option == QImageIOHandler::Description)
        m_description = value.toString();
}

QVariant QGIFLibHandler::option( ImageOption option ) const
{
    switch (option)
    {
    case QImageIOHandler::ImageFormat:
        return QVariant();	/// @todo
        break;
    case QImageIOHandler::Size:
        return QVariant();	/// @todo
        break;
    case QImageIOHandler::Description:
        return QVariant(m_description);
        break;
    default:
        return QVariant();
    }
}
