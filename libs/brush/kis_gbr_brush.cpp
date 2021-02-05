/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <sys/types.h>
#include <QtEndian>

#include "kis_gbr_brush.h"

#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPoint>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_image.h"

struct GimpBrushV1Header {
    quint32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
    quint32 version;      /*  brush file version #  */
    quint32 width;        /*  width of brush  */
    quint32 height;       /*  height of brush  */
    quint32 bytes;        /*  depth of brush in bytes */
};

/// All fields are in MSB on disk!
struct GimpBrushHeader {
    quint32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
    quint32 version;      /*  brush file version #  */
    quint32 width;        /*  width of brush  */
    quint32 height;       /*  height of brush  */
    quint32 bytes;        /*  depth of brush in bytes */

    /*  The following are only defined in version 2 */
    quint32 magic_number; /*  GIMP brush magic number  */
    quint32 spacing;      /*  brush spacing as % of width & height, 0 - 1000 */
};

// Needed, or the GIMP won't open it!
quint32 const GimpV2BrushMagic = ('G' << 24) + ('I' << 16) + ('M' << 8) + ('P' << 0);


struct KisGbrBrush::Private {

    QByteArray data;
    bool ownData;         /* seems to indicate that @ref data is owned by the brush, but in Qt4.x this is already guaranteed... so in reality it seems more to indicate whether the data is loaded from file (ownData = true) or memory (ownData = false) */

    quint32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
    quint32 version;      /*  brush file version #  */
    quint32 bytes;        /*  depth of brush in bytes */
    quint32 magic_number; /*  GIMP brush magic number  */
};

#define DEFAULT_SPACING 0.25

KisGbrBrush::KisGbrBrush(const QString& filename)
    : KisColorfulBrush(filename)
    , d(new Private)
{
    d->ownData = true;
    setSpacing(DEFAULT_SPACING);
}

KisGbrBrush::KisGbrBrush(const QString& filename,
                         const QByteArray& data,
                         qint32 & dataPos)
    : KisColorfulBrush(filename)
    , d(new Private)
{
    d->ownData = false;
    setSpacing(DEFAULT_SPACING);

    d->data = QByteArray::fromRawData(data.data() + dataPos, data.size() - dataPos);
    init();
    d->data.clear();
    dataPos += d->header_size + (width() * height() * d->bytes);
}

KisGbrBrush::KisGbrBrush(KisPaintDeviceSP image, int x, int y, int w, int h)
    : KisColorfulBrush()
    , d(new Private)
{
    d->ownData = true;
    setSpacing(DEFAULT_SPACING);
    initFromPaintDev(image, x, y, w, h);
}

KisGbrBrush::KisGbrBrush(const QImage& image, const QString& name)
    : KisColorfulBrush()
    , d(new Private)
{
    d->ownData = false;
    setSpacing(DEFAULT_SPACING);

    setBrushTipImage(image);
    setName(name);
}

KisGbrBrush::KisGbrBrush(const KisGbrBrush& rhs)
    : KisColorfulBrush(rhs)
    , d(new Private(*rhs.d))
{
    d->data = QByteArray();
}

KisGbrBrush::~KisGbrBrush()
{
    delete d;
}

bool KisGbrBrush::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    file.open(QIODevice::ReadOnly);
    bool res = loadFromDevice(&file);
    file.close();

    return res;
}

bool KisGbrBrush::loadFromDevice(QIODevice *dev)
{
    if (d->ownData) {
        d->data = dev->readAll();
    }
    return init();
}

bool KisGbrBrush::init()
{
    GimpBrushHeader bh;

    if (sizeof(GimpBrushHeader) > (uint)d->data.size()) {
        return false;
    }

    memcpy(&bh, d->data, sizeof(GimpBrushHeader));
    bh.header_size = qFromBigEndian(bh.header_size);
    d->header_size = bh.header_size;

    bh.version = qFromBigEndian(bh.version);
    d->version = bh.version;

    bh.width = qFromBigEndian(bh.width);
    bh.height = qFromBigEndian(bh.height);

    bh.bytes = qFromBigEndian(bh.bytes);
    d->bytes = bh.bytes;

    bh.magic_number = qFromBigEndian(bh.magic_number);
    d->magic_number = bh.magic_number;

    if (bh.version == 1) {
        // No spacing in version 1 files so use Gimp default
        bh.spacing = static_cast<int>(DEFAULT_SPACING * 100);
    }
    else {
        bh.spacing = qFromBigEndian(bh.spacing);

        if (bh.spacing > 1000) {
            return false;
        }
    }

    setSpacing(bh.spacing / 100.0);

    if (bh.header_size > (uint)d->data.size() || bh.header_size == 0) {
        return false;
    }

    QString name;

    if (bh.version == 1) {
        // Version 1 has no magic number or spacing, so the name
        // is at a different offset. Character encoding is undefined.
        const char *text = d->data.constData() + sizeof(GimpBrushV1Header);
        name = QString::fromLatin1(text, bh.header_size - sizeof(GimpBrushV1Header) - 1);
    }
    else {
        // ### Version = 3->cinepaint; may be float16 data!
        // Version >=2: UTF-8 encoding is used
        name = QString::fromUtf8(d->data.constData() + sizeof(GimpBrushHeader),
                                 bh.header_size - sizeof(GimpBrushHeader) - 1);
    }

    setName(name);

    if (bh.width == 0 || bh.height == 0) {
        return false;
    }

    QImage::Format imageFormat;

    if (bh.bytes == 1) {
        imageFormat = QImage::Format_Indexed8;
    } else {
        imageFormat = QImage::Format_ARGB32;
    }

    QImage image(QImage(bh.width, bh.height, imageFormat));

    if (image.isNull()) {
        return false;
    }

    qint32 k = bh.header_size;

    if (bh.bytes == 1) {
        QVector<QRgb> table;
        for (int i = 0; i < 256; ++i) table.append(qRgb(i, i, i));
        image.setColorTable(table);
        // Grayscale

        if (static_cast<qint32>(k + bh.width * bh.height) > d->data.size()) {
            return false;
        }

        setBrushApplication(ALPHAMASK);
        setBrushType(MASK);
        setHasColorAndTransparency(false);

        for (quint32 y = 0; y < bh.height; y++) {
            uchar *pixel = reinterpret_cast<uchar *>(image.scanLine(y));
            for (quint32 x = 0; x < bh.width; x++, k++) {
                qint32 val = 255 - static_cast<uchar>(d->data[k]);
                *pixel = val;
                ++pixel;
            }
        }
    } else if (bh.bytes == 4) {
        // RGBA

        if (static_cast<qint32>(k + (bh.width * bh.height * 4)) > d->data.size()) {
            return false;
        }

        setBrushApplication(IMAGESTAMP);
        setBrushType(IMAGE);

        for (quint32 y = 0; y < bh.height; y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (quint32 x = 0; x < bh.width; x++, k += 4) {
                *pixel = qRgba(d->data[k], d->data[k + 1], d->data[k + 2], d->data[k + 3]);
                ++pixel;
            }
        }

        setHasColorAndTransparency(!image.allGray());
    }
    else {
        warnKrita << "WARNING: loading of GBR brushes with" << bh.bytes << "bytes per pixel is not supported";
        return false;
    }

    setWidth(image.width());
    setHeight(image.height());
    if (d->ownData) {
        d->data.resize(0); // Save some memory, we're using enough of it as it is.
    }
    setValid(image.width() != 0 && image.height() != 0);
    setBrushTipImage(image);
    return true;
}

bool KisGbrBrush::initFromPaintDev(KisPaintDeviceSP image, int x, int y, int w, int h)
{
    // Forcefully convert to RGBA8
    // XXX profile and exposure?
    setBrushTipImage(image->convertToQImage(0, x, y, w, h, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags()));
    setName(image->objectName());

    setBrushType(IMAGE);
    setBrushApplication(IMAGESTAMP);

    return true;
}

bool KisGbrBrush::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    bool ok = saveToDevice(&file);
    file.close();
    return ok;
}

bool KisGbrBrush::saveToDevice(QIODevice* dev) const
{
    GimpBrushHeader bh;
    QByteArray utf8Name = name().toUtf8(); // Names in v2 brushes are in UTF-8
    char const* name = utf8Name.data();
    int nameLength = qstrlen(name);
    int wrote;

    bh.header_size = qToBigEndian((quint32)sizeof(GimpBrushHeader) + nameLength + 1);
    bh.version = qToBigEndian((quint32)2); // Only RGBA8 data needed atm, no cinepaint stuff
    bh.width = qToBigEndian((quint32)width());
    bh.height = qToBigEndian((quint32)height());
    // Hardcoded, 4 bytes RGBA or 1 byte GREY
    if (!isImageType()) {
        bh.bytes = qToBigEndian((quint32)1);
    }
    else {
        bh.bytes = qToBigEndian((quint32)4);
    }
    bh.magic_number = qToBigEndian((quint32)GimpV2BrushMagic);
    bh.spacing = qToBigEndian(static_cast<quint32>(spacing() * 100.0));

    // Write header: first bh, then the name
    QByteArray bytes = QByteArray::fromRawData(reinterpret_cast<char*>(&bh), sizeof(GimpBrushHeader));
    wrote = dev->write(bytes);
    bytes.clear();

    if (wrote == -1) {
        return false;
    }

    wrote = dev->write(name, nameLength + 1);

    if (wrote == -1) {
        return false;
    }

    int k = 0;

    QImage image = brushTipImage();

    if (!isImageType()) {
        bytes.resize(width() * height());
        for (qint32 y = 0; y < height(); y++) {
            for (qint32 x = 0; x < width(); x++) {
                QRgb c = image.pixel(x, y);
                bytes[k++] = static_cast<char>(255 - qRed(c)); // red == blue == green
            }
        }
    } else {
        bytes.resize(width() * height() * 4);
        for (qint32 y = 0; y < height(); y++) {
            for (qint32 x = 0; x < width(); x++) {
                // order for gimp brushes, v2 is: RGBA
                QRgb pixel = image.pixel(x, y);
                bytes[k++] = static_cast<char>(qRed(pixel));
                bytes[k++] = static_cast<char>(qGreen(pixel));
                bytes[k++] = static_cast<char>(qBlue(pixel));
                bytes[k++] = static_cast<char>(qAlpha(pixel));
            }
        }
    }

    wrote = dev->write(bytes);
    if (wrote == -1) {
        return false;
    }

    KoResource::saveToDevice(dev);

    return true;
}

void KisGbrBrush::setBrushTipImage(const QImage& image)
{
    KisBrush::setBrushTipImage(image);
    setValid(true);
}

void KisGbrBrush::makeMaskImage(bool preserveAlpha)
{
    if (!isImageType()) {
        return;
    }

    QImage brushTip = brushTipImage();

    if (!preserveAlpha) {
        const int imageWidth = brushTip.width();
        const int imageHeight = brushTip.height();
        QImage image(imageWidth, imageHeight, QImage::Format_Indexed8);
        QVector<QRgb> table;
        for (int i = 0; i < 256; ++i) {
            table.append(qRgb(i, i, i));
        }
        image.setColorTable(table);

        for (int y = 0; y < imageHeight; y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(brushTip.scanLine(y));
            uchar * dstPixel = image.scanLine(y);
            for (int x = 0; x < imageWidth; x++) {
                QRgb c = pixel[x];
                float alpha = qAlpha(c) / 255.0f;
                // linear interpolation with maximum gray value which is transparent in the mask
                //int a = (qGray(c) * alpha) + ((1.0 - alpha) * 255);
                // single multiplication version
                int a = 255 + alpha * (qGray(c) - 255);
                dstPixel[x] = (uchar)a;
            }
        }
        setBrushTipImage(image);
        setBrushType(MASK);
    }
    else {
        setBrushTipImage(brushTip);
        setBrushType(IMAGE);
    }

    setBrushApplication(ALPHAMASK);
    resetBoundary();
    clearBrushPyramid();
}

KisBrush* KisGbrBrush::clone() const
{
    return new KisGbrBrush(*this);
}

void KisGbrBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    predefinedBrushToXML("gbr_brush", e);
    KisColorfulBrush::toXML(d, e);
}

QString KisGbrBrush::defaultFileExtension() const
{
    return QString(".gbr");
}
