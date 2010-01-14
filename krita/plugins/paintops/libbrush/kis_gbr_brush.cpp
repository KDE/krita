/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_gbr_brush.h"
#include "kis_brush.h"
#include "kis_qimage_mask.h"

#include <QDomElement>
#include <QFile>
#include <QImage>
#include <QPoint>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"

#include <netinet/in.h> // htonl

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
    bool ownData;

    bool useColorAsMask;

    quint32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
    quint32 version;      /*  brush file version #  */
    quint32 bytes;        /*  depth of brush in bytes */
    quint32 magic_number; /*  GIMP brush magic number  */

};

#define DEFAULT_SPACING 0.25

KisGbrBrush::KisGbrBrush(const QString& filename)
    : KisBrush(filename)
    , d(new Private)
{
    setBrushType(INVALID);
    d->ownData = true;
    d->useColorAsMask = false;
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);
}

KisGbrBrush::KisGbrBrush(const QString& filename,
                         const QByteArray& data,
                         qint32 & dataPos)
                             : KisBrush(filename)
                             , d(new Private)
{
    setBrushType(INVALID);
    d->ownData = false;
    d->useColorAsMask = false;
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);

    d->data = QByteArray::fromRawData(data.data() + dataPos, data.size() - dataPos);
    init();
    d->data.clear();
    dataPos += d->header_size + (width() * height() * d->bytes);
}

KisGbrBrush::KisGbrBrush(KisPaintDeviceSP image, int x, int y, int w, int h)
    : KisBrush()
    , d(new Private)
{
    setBrushType(INVALID);
    d->ownData = true;
    d->useColorAsMask = false;
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);

    initFromPaintDev(image, x, y, w, h);
}

KisGbrBrush::KisGbrBrush(const QImage& image, const QString& name)
    : KisBrush()
    , d(new Private)
{
    d->ownData = false;
    d->useColorAsMask = false;
    setHasColor(false);
    setSpacing(DEFAULT_SPACING);

    setImage(image);
    setName(name);
    setBrushType(IMAGE);
}

KisGbrBrush::KisGbrBrush(const KisGbrBrush& rhs)
    : KisBrush(rhs)
    , d(new Private)
{
    setName(rhs.name());
    *d = *rhs.d;
    d->data = QByteArray();
    setValid(rhs.valid());
}

KisGbrBrush::~KisGbrBrush()
{
    delete d;
}

bool KisGbrBrush::load()
{
    if (d->ownData) {
        QFile file(filename());
        file.open(QIODevice::ReadOnly);
        d->data = file.readAll();
        file.close();
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
    bh.header_size = ntohl(bh.header_size);
    d->header_size = bh.header_size;

    bh.version = ntohl(bh.version);
    d->version = bh.version;

    bh.width = ntohl(bh.width);
    bh.height = ntohl(bh.height);

    bh.bytes = ntohl(bh.bytes);
    d->bytes = bh.bytes;

    bh.magic_number = ntohl(bh.magic_number);
    d->magic_number = bh.magic_number;

    if (bh.version == 1) {
        // No spacing in version 1 files so use Gimp default
        bh.spacing = static_cast<int>(DEFAULT_SPACING * 100);
    } else {
        bh.spacing = ntohl(bh.spacing);

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
        name = QString::fromAscii(text, bh.header_size - sizeof(GimpBrushV1Header) - 1);
    } else {
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
        imageFormat = QImage::Format_RGB32;
    } else {
        imageFormat = QImage::Format_ARGB32;
    }

    setImage(QImage(bh.width, bh.height, imageFormat));

    if (m_image.isNull()) {
        return false;
    }

    qint32 k = bh.header_size;

    if (bh.bytes == 1) {
        // Grayscale

        if (static_cast<qint32>(k + bh.width * bh.height) > d->data.size()) {
            return false;
        }

        setBrushType(MASK);
        setHasColor(false);

        for (quint32 y = 0; y < bh.height; y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(m_image.scanLine(y));
            for (quint32 x = 0; x < bh.width; x++, k++) {
                qint32 val = 255 - static_cast<uchar>(d->data[k]);
                *pixel = qRgb(val, val, val);
                ++pixel;
            }
        }
    } else if (bh.bytes == 4) {
        // RGBA

        if (static_cast<qint32>(k + (bh.width * bh.height * 4)) > d->data.size()) {
            return false;
        }

        setBrushType(IMAGE);
        setHasColor(true);

        for (quint32 y = 0; y < bh.height; y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(m_image.scanLine(y));
            for (quint32 x = 0; x < bh.width; x++, k += 4) {
                *pixel = qRgba(d->data[k], d->data[k+1], d->data[k+2], d->data[k+3]);
                ++pixel;
            }
        }
    } else {
        return false;
    }

    setWidth(m_image.width());
    setHeight(m_image.height());
    if (d->ownData) {
        d->data.resize(0); // Save some memory, we're using enough of it as it is.
    }


    if (m_image.width() == 0 || m_image.height() == 0)
        setValid(false);
    else
        setValid(true);

    return true;
}

bool KisGbrBrush::initFromPaintDev(KisPaintDeviceSP image, int x, int y, int w, int h)
{
    // Forcefully convert to RGBA8
    // XXX profile and exposure?
    setImage(image->convertToQImage(0, x, y, w, h));
    setName(image->objectName());

    setBrushType(IMAGE);
    setHasColor(true);

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

    bh.header_size = htonl(sizeof(GimpBrushHeader) + nameLength);
    bh.version = htonl(2); // Only RGBA8 data needed atm, no cinepaint stuff
    bh.width = htonl(width());
    bh.height = htonl(height());
    // Hardcoded, 4 bytes RGBA or 1 byte GREY
    if (!hasColor())
        bh.bytes = htonl(1);
    else
        bh.bytes = htonl(4);
    bh.magic_number = htonl(GimpV2BrushMagic);
    bh.spacing = htonl(static_cast<quint32>(spacing() * 100.0));

    // Write header: first bh, then the name
    QByteArray bytes = QByteArray::fromRawData(reinterpret_cast<char*>(&bh), sizeof(GimpBrushHeader));
    wrote = dev->write(bytes);
    bytes.clear();

    if (wrote == -1)
        return false;

    wrote = dev->write(name, nameLength); // No +1 for the trailing NULL it seems...
    if (wrote == -1)
        return false;

    int k = 0;

    if (!hasColor()) {
        bytes.resize(width() * height());
        for (qint32 y = 0; y < height(); y++) {
            for (qint32 x = 0; x < width(); x++) {
                QRgb c = m_image.pixel(x, y);
                bytes[k++] = static_cast<char>(255 - qRed(c)); // red == blue == green
            }
        }
    } else {
        bytes.resize(width() * height() * 4);
        for (qint32 y = 0; y < height(); y++) {
            for (qint32 x = 0; x < width(); x++) {
                // order for gimp brushes, v2 is: RGBA
                QRgb pixel = m_image.pixel(x, y);
                bytes[k++] = static_cast<char>(qRed(pixel));
                bytes[k++] = static_cast<char>(qGreen(pixel));
                bytes[k++] = static_cast<char>(qBlue(pixel));
                bytes[k++] = static_cast<char>(qAlpha(pixel));
            }
        }
    }

    wrote = dev->write(bytes);
    if (wrote == -1)
        return false;

    return true;
}

QImage KisGbrBrush::image() const
{
    if (hasColor() && useColorAsMask()) {
        QImage image = m_image;

        for (int y = 0; y < image.height(); y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                QRgb c = pixel[x];
                int a = (qGray(c) * qAlpha(c)) / 255;
                pixel[x] = qRgba(a, 0, a, a);
            }
        }
        return image;
    } else {
        return m_image;
    }
}


enumBrushType KisGbrBrush::brushType() const
{
    if (KisBrush::brushType() == IMAGE && useColorAsMask()) {
        return MASK;
    } else {
        return KisBrush::brushType();
    }
}


void KisGbrBrush::setImage(const QImage& image)
{
    KisBrush::setImage(image);
    setValid(true);
}

/*QImage KisGbrBrush::outline(double pressure) {
    KisLayerSP layer = image(KoColorSpaceRegistry::instance()->colorSpace("RGBA",0),
                             KisPaintInformation(pressure));
    KisBoundary bounds(layer.data());
    int w = maskWidth(pressure);
    int h = maskHeight(pressure);

    bounds.generateBoundary(w, h);
    QPixmap pix(bounds.pixmap(w, h));
    QImage result;
    result = pix;
    return result;
}*/

void KisGbrBrush::makeMaskImage()
{
    if (!hasColor())
        return;

    QImage image(width(), height(), QImage::Format_RGB32);

    if (m_image.width() == image.width() && m_image.height() == image.height()) {

        for (int y = 0; y < height(); y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(m_image.scanLine(y));
            for (int x = 0; x < width(); x++) {
                QRgb c = pixel[x];
                int a = (qGray(c) * qAlpha(c)) / 255; // qGray(black) = 0
                pixel[x] = qRgba(a, a, a, 255);
            }
        }

        setImage(image);
    }

    setBrushType(MASK);
    setHasColor(false);
    d->useColorAsMask = false;
    resetBoundary();
    clearScaledBrushes();
}

KisGbrBrush* KisGbrBrush::clone() const
{
    return new KisGbrBrush(*this);
}

void KisGbrBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    Q_UNUSED(d);
    e.setAttribute("type", "brush"); // legacy
    e.setAttribute("brush_type", "kis_gbr_brush");
    e.setAttribute("name", name()); // legacy
    e.setAttribute("filename", filename()); // legacy
    e.setAttribute("brush_filename", filename());
    e.setAttribute("brush_spacing", spacing());
}

void KisGbrBrush::setUseColorAsMask(bool useColorAsMask)
{
    d->useColorAsMask = useColorAsMask;
}
bool KisGbrBrush::useColorAsMask() const
{
    return d->useColorAsMask;
}

QString KisGbrBrush::defaultFileExtension() const
{
    return QString(".gbr");
}
