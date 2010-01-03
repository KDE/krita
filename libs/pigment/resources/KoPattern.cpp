
/*  This file is part of the KDE project

    Copyright (c) 2000 Matthias Elter <elter@kde.org>
    Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoPattern.h"

#include <sys/types.h>
#include <netinet/in.h>

#include <QImage>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

namespace
{
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

KoPattern::KoPattern(const QString& file)
        : KoResource(file)
{
}

KoPattern::~KoPattern()
{
}

bool KoPattern::load()
{
    QString fileExtension;
    int index = filename().lastIndexOf('.');

    if (index != -1)
        fileExtension = filename().mid(index).toLower();

    bool result;
    if (fileExtension == ".pat") {
        QFile file(filename());
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        file.close();
        result = init(data);
    } else {
        result = m_image.load(filename());
        setValid(result);
    }
    return result;
}

bool KoPattern::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);

    QTextStream stream(&file);
    // Header: header_size (24+name length),version,width,height,colordepth of brush,magic,name
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
            QRgb pixel = m_image.pixel(x, y);
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

QImage KoPattern::image() const
{
    return m_image;
}

bool KoPattern::init(QByteArray& bytes)
{
    int dataSize = bytes.size();
    const char* data = bytes.constData();

    // load Gimp patterns
    GimpPatternHeader bh;
    qint32 k;
    char* name;

    if ((int)sizeof(GimpPatternHeader) > dataSize) {
        return false;
    }

    memcpy(&bh, data, sizeof(GimpPatternHeader));
    bh.header_size = ntohl(bh.header_size);
    bh.version = ntohl(bh.version);
    bh.width = ntohl(bh.width);
    bh.height = ntohl(bh.height);
    bh.bytes = ntohl(bh.bytes);
    bh.magic_number = ntohl(bh.magic_number);

    if ((int)bh.header_size > dataSize || bh.header_size == 0) {
        return false;
    }
    int size = bh.header_size - sizeof(GimpPatternHeader);
    name = new char[size];
    memcpy(name, data + sizeof(GimpPatternHeader), size);

    if (name[size - 1]) {
        delete[] name;
        return false;
    }

    setName(QString::fromAscii(name, size));
    delete[] name;

    if (bh.width == 0 || bh.height == 0) {
        return false;
    }

    QImage::Format imageFormat;

    if (bh.bytes == 1 || bh.bytes == 3) {
        imageFormat = QImage::Format_RGB32;
    } else {
        imageFormat = QImage::Format_ARGB32;
    }

    m_image = QImage(bh.width, bh.height, imageFormat);

    if (m_image.isNull()) {
        return false;
    }

    k = bh.header_size;

    if (bh.bytes == 1) {
        // Grayscale
        qint32 val;

        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++, k++) {
                if (k > dataSize) {
                    kWarning(30009) << "failed in gray";
                    return false;
                }

                val = data[k];
                m_image.setPixel(x, y, qRgb(val, val, val));
            }
        }
    } else if (bh.bytes == 2) {
        // Grayscale + A
        qint32 val;
        qint32 alpha;
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++, k++) {
                if (k + 2 > dataSize) {
                    kWarning(30009) << "failed in grayA";
                    return false;
                }

                val = data[k];
                alpha = data[k++];
                m_image.setPixel(x, y, qRgba(val, val, val, alpha));
            }
        }
    } else if (bh.bytes == 3) {
        // RGB without alpha
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++) {
                if (k + 3 > dataSize) {
                    kWarning(30009) << "failed in RGB";
                    return false;
                }

                m_image.setPixel(x, y, qRgb(data[k],
                                            data[k + 1],
                                            data[k + 2]));
                k += 3;
            }
        }
    } else if (bh.bytes == 4) {
        // Has alpha
        for (quint32 y = 0; y < bh.height; y++) {
            for (quint32 x = 0; x < bh.width; x++) {
                if (k + 4 > dataSize) {
                    kWarning(30009) << "failed in RGBA";
                    return false;
                }

                m_image.setPixel(x, y, qRgba(data[k],
                                             data[k + 1],
                                             data[k + 2],
                                             data[k + 3]));
                k += 4;
            }
        }
    } else {
        return false;
    }

    if (m_image.isNull()) {
        return false;
    }

    setValid(true);

    return true;
}

qint32 KoPattern::width() const
{
    return m_image.width();
}

qint32 KoPattern::height() const
{
    return m_image.height();
}

void KoPattern::setImage(const QImage& image)
{
    m_image = image;
    m_image.detach();

    setValid(true);
}

KoPattern& KoPattern::operator=(const KoPattern & pattern)
{
    setFilename(pattern.filename());
    setImage(pattern.image());
    setValid(true);
    return *this;
}

QString KoPattern::defaultFileExtension() const
{
    return QString(".pat");
}
