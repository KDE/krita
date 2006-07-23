/*
 *  kis_pattern.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_pattern.h"

#include <sys/types.h>
#include <netinet/in.h>

#include <limits.h>
#include <stdlib.h>

#include <QPoint>
#include <QSize>
#include <QImage>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include "KoColor.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

namespace {
    struct GimpPatternHeader {
        quint32 header_size;  /*  header_size = sizeof (PatternHeader) + brush name  */
        quint32 version;      /*  pattern file version #  */
        quint32 width;        /*  width of pattern */
        quint32 height;       /*  height of pattern  */
        quint32 bytes;        /*  depth of pattern in bytes : 1, 2, 3 or 4*/
        quint32 magic_number; /*  GIMP brush magic number  */
    };

    // Yes! This is _NOT_ what my pat.txt file says. It's really not 'GIMP', but 'GPAT'
    quint32 const GimpPatternMagic = (('G' << 24) + ('P' << 16) + ('A' << 8) + ('T' << 0));
}

KisPattern::KisPattern(const QString& file) : super(file), m_hasFile(true)
{
}

KisPattern::KisPattern(KisPaintDevice* image, int x, int y, int w, int h)
    : super(""), m_hasFile(false)
{
    // Forcefully convert to RGBA8
    // XXX profile and exposure?
    setImage(image->convertToQImage(0, x, y, w, h));
    setName(image->objectName());
}

KisPattern::~KisPattern()
{
}

bool KisPattern::load()
{
    if (!m_hasFile)
        return true;

    QFile file(filename());
    file.open(QIODevice::ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}

bool KisPattern::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);

    QTextStream stream(&file);
    // Header: header_size (24+name length),version,width,height,colourdepth of brush,magic,name
    // depth: 1 = greyscale, 2 = greyscale + A, 3 = RGB, 4 = RGBA
    // magic = "GPAT", as a single uint32, the docs are wrong here!
    // name is UTF-8 (\0-terminated! The docs say nothing about this!)
    // _All_ data in network order, it seems! (not mentioned in gimp-2.2.8/devel-docs/pat.txt!!)
    // We only save RGBA at the moment
    // Version is 1 for now...

    GimpPatternHeader ph;
    QByteArray utf8Name = name().toUtf8();
    char const* name = utf8Name.data();
    int nameLength = qstrlen(name);

    ph.header_size = htonl(sizeof(GimpPatternHeader) + nameLength + 1); // trailing 0
    ph.version = htonl(1);
    ph.width = htonl(width());
    ph.height = htonl(height());
    ph.bytes = htonl(4);
    ph.magic_number = htonl(GimpPatternMagic);

    QByteArray bytes = QByteArray::fromRawData(reinterpret_cast<char*>(&ph), sizeof(GimpPatternHeader));
    int wrote = file.write(bytes);
    bytes.clear();

    if (wrote == -1)
        return false;

    wrote = file.write(name, nameLength + 1); // Trailing 0 apparantly!
    if (wrote == -1)
        return false;

    int k = 0;
    bytes.resize(width() * height() * 4);
    for (qint32 y = 0; y < height(); y++) {
        for (qint32 x = 0; x < width(); x++) {
            // RGBA only
            QRgb pixel = m_img.pixel(x,y);
            bytes[k++] = static_cast<char>(qRed(pixel));
            bytes[k++] = static_cast<char>(qGreen(pixel));
            bytes[k++] = static_cast<char>(qBlue(pixel));
            bytes[k++] = static_cast<char>(qAlpha(pixel));
        }
    }

    wrote = file.write(bytes);
    if (wrote == -1)
        return false;

    file.close();

    return true;
}

QImage KisPattern::img()
{
    return m_img;
}

bool KisPattern::init()
{
    // load Gimp patterns
    GimpPatternHeader bh;
    qint32 k;
    QByteArray name;

    if ((int)sizeof(GimpPatternHeader) > m_data.size()) {
        return false;
    }

    memcpy(&bh, m_data, sizeof(GimpPatternHeader));
    bh.header_size = ntohl(bh.header_size);
    bh.version = ntohl(bh.version);
    bh.width = ntohl(bh.width);
    bh.height = ntohl(bh.height);
    bh.bytes = ntohl(bh.bytes);
    bh.magic_number = ntohl(bh.magic_number);

    if ((int)bh.header_size > m_data.size() || bh.header_size == 0) {
        return false;
    }

    name.resize(bh.header_size - sizeof(GimpPatternHeader));
    memcpy(name.data(), m_data.constData() + sizeof(GimpPatternHeader), name.size());

    if (name[name.size() - 1]) {
        return false;
    }

    setName(i18n(name));

    if (bh.width == 0 || bh.height == 0) {
        return false;
    }

    QImage::Format imageFormat;

    if (bh.bytes == 1 || bh.bytes == 3) {
        imageFormat = QImage::Format_RGB32;
    } else {
        imageFormat = QImage::Format_ARGB32;
    }

    m_img = QImage(bh.width, bh.height, imageFormat);

    if (m_img.isNull()) {
        return false;
    }

    k = bh.header_size;

    if (bh.bytes == 1) {
        // Grayscale
        qint32 val;

        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++, k++) {
                if (k > m_data.size()) {
                    kDebug(DBG_AREA_FILE) << "failed in gray\n";
                    return false;
                }

                val = m_data[k];
                m_img.setPixel(x, y, qRgb(val, val, val));
            }
        }
    } else if (bh.bytes == 2) {
        // Grayscale + A
        qint32 val;
        qint32 alpha;
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++, k++) {
                if (k + 2 > m_data.size()) {
                    kDebug(DBG_AREA_FILE) << "failed in grayA\n";
                    return false;
                }

                val = m_data[k];
                alpha = m_data[k++];
                m_img.setPixel(x, y, qRgba(val, val, val, alpha));
            }
        }
    } else if (bh.bytes == 3) {
        // RGB without alpha
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++) {
                if (k + 3 > m_data.size()) {
                    kDebug(DBG_AREA_FILE) << "failed in RGB\n";
                    return false;
                }

                m_img.setPixel(x, y, qRgb(m_data[k],
                              m_data[k + 1],
                              m_data[k + 2]));
                k += 3;
            }
        }
    } else if (bh.bytes == 4) {
        // Has alpha
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++) {
                if (k + 4 > m_data.size()) {
                    kDebug(DBG_AREA_FILE) << "failed in RGBA\n";
                    return false;
                }

                m_img.setPixel(x, y, qRgba(m_data[k],
                               m_data[k + 1],
                               m_data[k + 2],
                               m_data[k + 3]));
                k += 4;
            }
        }
    } else {
        return false;
    }

    if (m_img.isNull()) {
        return false;
    }

    setWidth(m_img.width());
    setHeight(m_img.height());

    setValid(true);

    return true;
}

KisPaintDeviceSP KisPattern::image(KoColorSpace * colorSpace) {
    // Check if there's already a pattern prepared for this colorspace
    QMap<QString, KisPaintDeviceSP>::const_iterator it = m_colorspaces.find(colorSpace->id());
    if (it != m_colorspaces.end())
        return (*it);

    // If not, create one
    KisPaintDeviceSP layer = KisPaintDeviceSP(new KisPaintDevice(colorSpace, "pattern"));

    Q_CHECK_PTR(layer);

    layer->convertFromQImage(m_img,"");

    m_colorspaces[colorSpace->id()] = layer;
    return layer;
}

qint32 KisPattern::width() const
{
    return m_width;
}

void KisPattern::setWidth(qint32 w)
{
    m_width = w;
}

qint32 KisPattern::height() const
{
    return m_height;
}

void KisPattern::setHeight(qint32 h)
{
    m_height = h;
}

void KisPattern::setImage(const QImage& img)
{
    m_hasFile = false;
    m_img = img;
    m_img.detach();

    setWidth(img.width());
    setHeight(img.height());

    setValid(true);
}

KisPattern* KisPattern::clone() const
{
    KisPattern* pattern = new KisPattern("");
    pattern->setImage(m_img);
    pattern->setName(name());
    return pattern;
}

#include "kis_pattern.moc"
