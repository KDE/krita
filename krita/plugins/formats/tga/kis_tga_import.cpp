/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tga_import.h"

#include <QCheckBox>
#include <QBuffer>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>
#include <QUrl>

#include <KoColorSpace.h>
#include <KisFilterChain.h>
#include <KoColorSpaceRegistry.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include <tga.h>

K_PLUGIN_FACTORY_WITH_JSON(KisTGAImportFactory, "krita_tga_import.json", registerPlugin<KisTGAImport>();)

KisTGAImport::KisTGAImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTGAImport::~KisTGAImport()
{
}

static QDataStream & operator>> (QDataStream & s, TgaHeader & head)
{
    s >> head.id_length;
    s >> head.colormap_type;
    s >> head.image_type;
    s >> head.colormap_index;
    s >> head.colormap_length;
    s >> head.colormap_size;
    s >> head.x_origin;
    s >> head.y_origin;
    s >> head.width;
    s >> head.height;
    s >> head.pixel_size;
    s >> head.flags;

    /*dbgKrita << "id_length: " << head.id_length << " - colormap_type: " << head.colormap_type << " - image_type: " << head.image_type;
    dbgKrita << "colormap_index: " << head.colormap_index << " - colormap_length: " << head.colormap_length << " - colormap_size: " << head.colormap_size;
    dbgKrita << "x_origin: " << head.x_origin << " - y_origin: " << head.y_origin << " - width:" << head.width << " - height:" << head.height << " - pixelsize: " << head.pixel_size << " - flags: " << head.flags;*/

    return s;
}


static bool isSupported(const TgaHeader & head)
{
    if (head.image_type != TGA_TYPE_INDEXED &&
            head.image_type != TGA_TYPE_RGB &&
            head.image_type != TGA_TYPE_GREY &&
            head.image_type != TGA_TYPE_RLE_INDEXED &&
            head.image_type != TGA_TYPE_RLE_RGB &&
            head.image_type != TGA_TYPE_RLE_GREY) {
        return false;
    }

    if (head.image_type == TGA_TYPE_INDEXED ||
            head.image_type == TGA_TYPE_RLE_INDEXED) {
        if (head.colormap_length > 256 || head.colormap_size != 24 || head.colormap_type != 1) {
            return false;
        }
    }

    if (head.image_type == TGA_TYPE_RGB ||
            head.image_type == TGA_TYPE_GREY ||
            head.image_type == TGA_TYPE_RLE_RGB ||
            head.image_type == TGA_TYPE_RLE_GREY) {
        if (head.colormap_type != 0) {
            return false;
        }
    }

    if (head.width == 0 || head.height == 0) {
        return false;
    }

    if (head.pixel_size != 8 && head.pixel_size != 16 &&
            head.pixel_size != 24 && head.pixel_size != 32) {
        return false;
    }

    return true;
}

static bool loadTGA(QDataStream & s, const TgaHeader & tga, QImage &img)
{
    // Create image.
    img = QImage(tga.width, tga.height, QImage::Format_RGB32);

    TgaHeaderInfo info(tga);

    // However alpha exists only in the 32 bit format.
    if ((tga.pixel_size == 32) && (tga.flags & 0xf)) {
        img = QImage(tga.width, tga.height, QImage::Format_ARGB32);
    }

    uint pixel_size = (tga.pixel_size / 8);
    uint size = tga.width * tga.height * pixel_size;

    if (size < 1) {
        dbgFile << "This TGA file is broken with size " << size;
        return false;
    }

    // Read palette.
    char palette[768];
    if (info.pal) {
        // @todo Support palettes in other formats!
        s.readRawData(palette, 3 * tga.colormap_length);
    }

    // Allocate image.
    uchar * const image = new uchar[size];

    if (info.rle) {
        // Decode image.
        char * dst = (char *)image;
        int num = size;

        while (num > 0) {
            // Get packet header.
            uchar c;
            s >> c;

            uint count = (c & 0x7f) + 1;
            num -= count * pixel_size;

            if (c & 0x80) {
                // RLE pixels.
                Q_ASSERT(pixel_size <= 8);
                char pixel[8];
                s.readRawData(pixel, pixel_size);
                do {
                    memcpy(dst, pixel, pixel_size);
                    dst += pixel_size;
                } while (--count);
            } else {
                // Raw pixels.
                count *= pixel_size;
                s.readRawData(dst, count);
                dst += count;
            }
        }
    } else {
        // Read raw image.
        s.readRawData((char *)image, size);
    }

    // Convert image to internal format.
    int y_start, y_step, y_end;
    if (tga.flags & TGA_ORIGIN_UPPER) {
        y_start = 0;
        y_step = 1;
        y_end = tga.height;
    } else {
        y_start = tga.height - 1;
        y_step = -1;
        y_end = -1;
    }

    uchar* src = image;

    for (int y = y_start; y != y_end; y += y_step) {
        QRgb * scanline = (QRgb *) img.scanLine(y);

        if (info.pal) {
            // Paletted.
            for (int x = 0; x < tga.width; x++) {
                uchar idx = *src++;
                scanline[x] = qRgb(palette[3 * idx + 2], palette[3 * idx + 1], palette[3 * idx + 0]);
            }
        } else if (info.grey) {
            // Greyscale.
            for (int x = 0; x < tga.width; x++) {
                scanline[x] = qRgb(*src, *src, *src);
                src++;
            }
        } else {
            // True Color.
            if (tga.pixel_size == 16) {
                for (int x = 0; x < tga.width; x++) {
                    Color555 c = *reinterpret_cast<Color555 *>(src);
                    scanline[x] = qRgb((c.r << 3) | (c.r >> 2), (c.g << 3) | (c.g >> 2), (c.b << 3) | (c.b >> 2));
                    src += 2;
                }
            } else if (tga.pixel_size == 24) {
                for (int x = 0; x < tga.width; x++) {
                    scanline[x] = qRgb(src[2], src[1], src[0]);
                    src += 3;
                }
            } else if (tga.pixel_size == 32) {
                for (int x = 0; x < tga.width; x++) {
                    const uchar alpha = src[3];
                    scanline[x] = qRgba(src[2], src[1], src[0], alpha);
                    src += 4;
                }
            }
        }
    }

    // Free image.
    delete []image;

    return true;
}



KisImportExportFilter::ConversionStatus KisTGAImport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "TGA import! From:" << from << ", To:" << to << 0;

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    KisDocument * doc = m_chain->outputDocument();

    if (!doc)
        return KisImportExportFilter::NoDocumentCreated;

    QString filename = m_chain->inputFile();

    doc->prepareForImport();

    if (!filename.isEmpty()) {
        QUrl url(filename);

        if (url.isEmpty())
            return KisImportExportFilter::FileNotFound;

        if (!url.isLocalFile()) {
            return KisImportExportFilter::FileNotFound;
        }


        QString localFile = url.toLocalFile();

        QFile f(localFile);
        f.open(QIODevice::ReadOnly);
        QDataStream s(&f);
        s.setByteOrder(QDataStream::LittleEndian);

        TgaHeader tga;
        s >> tga;
        s.device()->seek(TgaHeader::SIZE + tga.id_length);


        // Check image file format.
        if (s.atEnd()) {
            return KisImportExportFilter::InvalidFormat;
        }

        // Check supported file types.
        if (!isSupported(tga)) {
            return KisImportExportFilter::InvalidFormat;
        }

        QImage img;
        bool result = loadTGA(s, tga, img);

        if (result == false) {
            return KisImportExportFilter::CreationError;
        }

        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(doc->createUndoStore(), img.width(), img.height(), colorSpace, "imported from tga");

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);
        layer->paintDevice()->convertFromQImage(img, 0, 0, 0);
        image->addNode(layer.data(), image->rootLayer().data());

        doc->setCurrentImage(image);
        return KisImportExportFilter::OK;
    }
    return KisImportExportFilter::StorageCreationError;

}

#include "kis_tga_import.moc"

