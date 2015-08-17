/*  This file is part of the KDE project
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "KoColorSet.h"

#include <sys/types.h>
#include <netinet/in.h> // htonl

#include <QImage>
#include <QPoint>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QBuffer>
#include <QByteArray>
#include <QPainter>

#include <kdebug.h>
#include <klocale.h>

#include "KoColor.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorModelStandardIds.h"


KoColorSet::PaletteType detectFormat(const QString &fileName, const QByteArray &ba) {

    QFileInfo fi(fileName);

    // .pal
    if (ba.startsWith("RIFF") && ba.indexOf("PAL data", 8)) {
        return KoColorSet::RIFF_PAL;
    }
    // .gpl
    else if (ba.startsWith("GIMP Palette")) {
        return KoColorSet::GPL;
    }
    // .pal
    else if (ba.startsWith("JASC-PAL")) {
        return KoColorSet::PSP_PAL;
    }
    else if (fi.suffix().toLower() == "aco") {
        return KoColorSet::ACO;
    }
    else if (fi.suffix().toLower() == "act") {
        return KoColorSet::ACT;
    }

    return KoColorSet::UNKNOWN;
}

KoColorSet::KoColorSet(const QString& filename)
    : KoResource(filename)
{
    // Implemented in KoResource class
    m_columns = 0; // Set the default value that the GIMP uses...
}

KoColorSet::KoColorSet()
    : KoResource("")
{
    m_columns = 0; // Set the default value that the GIMP uses...
}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : QObject(0)
    , KoResource("")
{
    setFilename(rhs.filename());
    m_ownData = false;
    m_name = rhs.m_name;
    m_comment = rhs.m_comment;
    m_columns = rhs.m_columns;
    m_colors = rhs.m_colors;
    setValid(true);
}

KoColorSet::~KoColorSet()
{
}

bool KoColorSet::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        kWarning() << "Can't open file " << filename();
        return false;
    }
    bool res =  loadFromDevice(&file);
    file.close();
    return res;
}

bool KoColorSet::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    m_data = dev->readAll();

    Q_ASSERT(m_data.size() != 0);

    return init();
}


bool KoColorSet::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();
    return true;
}

qint32 KoColorSet::nColors()
{
    return m_colors.count();
}

bool KoColorSet::saveToDevice(QIODevice *dev) const
{
    QTextStream stream(dev);
    stream << "GIMP Palette\nName: " << name() << "\nColumns: " << m_columns << "\n#\n";

    for (int i = 0; i < m_colors.size(); i++) {
        const KoColorSetEntry& entry = m_colors.at(i);
        QColor c = entry.color.toQColor();
        stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
        if (entry.name.isEmpty())
            stream << "Untitled\n";
        else
            stream << entry.name << "\n";
    }

    KoResource::saveToDevice(dev);

    return true;
}

bool KoColorSet::init()
{
    m_colors.clear(); // just in case this is a reload (eg by KoEditColorSetDialog),

    if (filename().isNull()) {
        qWarning() << "Cannot load palette" << name() << "there is no filename set";
        return false;
    }
    if (m_data.isNull()) {
        QFile file(filename());
        if (file.size() == 0) {
            qWarning() << "Cannot load palette" << name() << "there is no data available";
            return false;
        }
        file.open(QIODevice::ReadOnly);
        m_data = file.readAll();
        file.close();
    }

    bool res = false;
    PaletteType paletteType = detectFormat(filename(), m_data);
    switch(paletteType) {
    case GPL:
        res = loadGpl();
        break;
    case ACT:
        res = loadAct();
        break;
    case RIFF_PAL:
        res = loadRiff();
        break;
    case PSP_PAL:
        res = loadPsp();
        break;
    case ACO:
        res = loadAco();
        break;
    default:
        res = false;
    }
    setValid(res);

    if (m_columns == 0) {
        m_columns = 10;
    }

    QImage img(m_columns * 4, (m_colors.size() / m_columns) * 4, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(img.rect(), Qt::darkGray);
    int counter = 0;
    for(int i = 0; i < m_columns; ++i) {
        for (int j = 0; j < (m_colors.size() / m_columns); ++j) {
            if (counter < m_colors.size()) {
                QColor c = m_colors.at(counter).color.toQColor();
                gc.fillRect(i * 4, j * 4, 4, 4, c);
                counter++;
            }
            else {
                break;
            }
        }
    }
    setImage(img);

    // save some memory
    m_data.clear();
    return res;
}

void KoColorSet::add(const KoColorSetEntry & c)
{
    m_colors.push_back(c);
}

void KoColorSet::remove(const KoColorSetEntry & c)
{
    QVector<KoColorSetEntry>::iterator it = m_colors.begin();
    QVector<KoColorSetEntry>::iterator end = m_colors.end();

    while (it != end) {
        if ((*it) == c) {
            m_colors.erase(it);
            return;
        }
        ++it;
    }
}

void KoColorSet::removeAt(quint32 index)
{
    m_colors.remove(index);
}

KoColorSetEntry KoColorSet::getColor(quint32 index)
{
    return m_colors[index];
}

void KoColorSet::setColumnCount(int columns)
{
    m_columns = columns;
}

int KoColorSet::columnCount()
{
    return m_columns;
}

QString KoColorSet::defaultFileExtension() const
{
    return QString(".gpl");
}


bool KoColorSet::loadGpl()
{
    QString s = QString::fromUtf8(m_data.data(), m_data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
        kWarning(30009) << "Illegal Gimp palette file: " << filename();
        return false;
    }

    quint32 index = 0;

    QStringList lines = s.split('\n', QString::SkipEmptyParts);

    if (lines.size() < 3) {
        return false;
    }

    QString columns;
    qint32 r, g, b;
    KoColorSetEntry e;

    // Read name
    if (!lines[0].startsWith("GIMP") || !lines[1].startsWith("Name: ")) {
        kWarning(30009) << "Illegal Gimp palette file: " << filename();
        return false;
    }

    setName(i18n(lines[1].mid(strlen("Name: ")).trimmed().toLatin1()));

    index = 2;

    // Read columns
    if (lines[index].startsWith("Columns: ")) {
        columns = lines[index].mid(strlen("Columns: ")).trimmed();
        m_columns = columns.toInt();
        index = 3;
    }
    for (qint32 i = index; i < lines.size(); i++) {
        if (lines[i].startsWith('#')) {
            m_comment += lines[i].mid(1).trimmed() + ' ';
        } else if (!lines[i].isEmpty()) {
            QStringList a = lines[i].replace('\t', ' ').split(' ', QString::SkipEmptyParts);

            if (a.count() < 3) {
                break;
            }

            r = a[0].toInt();
            a.pop_front();
            g = a[0].toInt();
            a.pop_front();
            b = a[0].toInt();
            a.pop_front();

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            e.color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
            e.color.fromQColor(QColor(r, g, b));

            QString name = a.join(" ");
            e.name = name.isEmpty() ? i18n("Untitled") : name;

            add(e);
        }
    }
    return true;
}

bool KoColorSet::loadAct()
{
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;
    for (int i = 0; i < m_data.size(); i += 3) {
        quint8 r = m_data[i];
        quint8 g = m_data[i+1];
        quint8 b = m_data[i+2];
        e.color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
        e.color.fromQColor(QColor(r, g, b));
        add(e);
    }
    return true;
}

struct RiffHeader {
    quint32 riff;
    quint32 size;
    quint32 signature;
    quint32 data;
    quint32 datasize;
    quint16 version;
    quint16 colorcount;
};


bool KoColorSet::loadRiff()
{
    // http://worms2d.info/Palette_file
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;

    RiffHeader header;
    memcpy(&header, m_data.constData(), sizeof(RiffHeader));
    header.colorcount = ntohl(header.colorcount);

    for (int i = sizeof(RiffHeader);
         (i < (int)(sizeof(RiffHeader) + header.colorcount) && i < m_data.size());
         i += 4) {
        quint8 r = m_data[i];
        quint8 g = m_data[i+1];
        quint8 b = m_data[i+2];
        e.color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
        e.color.fromQColor(QColor(r, g, b));
        add(e);
    }
    return true;
}


bool KoColorSet::loadPsp()
{
    QFileInfo info(filename());
    setName(info.baseName());
    KoColorSetEntry e;
    qint32 r, g, b;

    QString s = QString::fromUtf8(m_data.data(), m_data.count());
    QStringList l = s.split('\n', QString::SkipEmptyParts);
    if (l.size() < 4) return false;
    if (l[0] != "JASC-PAL") return false;
    if (l[1] != "0100") return false;

    int entries = l[2].toInt();

    for (int i = 0; i < entries; ++i)  {

        QStringList a = l[i + 3].replace('\t', ' ').split(' ', QString::SkipEmptyParts);

        if (a.count() != 3) {
            continue;
        }

        r = a[0].toInt();
        a.pop_front();
        g = a[0].toInt();
        a.pop_front();
        b = a[0].toInt();
        a.pop_front();

        r = qBound(0, r, 255);
        g = qBound(0, g, 255);
        b = qBound(0, b, 255);

        e.color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
        e.color.fromQColor(QColor(r, g, b));

        QString name = a.join(" ");
        e.name = name.isEmpty() ? i18n("Untitled") : name;

        add(e);
    }
    return true;
}

quint16 readShort(QIODevice *io) {
    quint16 val;
    quint64 read = io->read((char*)&val, 2);
    if (read != 2) return false;
    return ntohs(val);
}

bool KoColorSet::loadAco()
{
    QFileInfo info(filename());
    setName(info.baseName());

    QBuffer buf(&m_data);
    buf.open(QBuffer::ReadOnly);

    quint16 version = readShort(&buf);
    quint16 numColors = readShort(&buf);
    KoColorSetEntry e;

    const quint16 quint16_MAX = 65535;

    for (int i = 0; i < numColors && !buf.atEnd(); ++i) {

        quint16 colorSpace = readShort(&buf);
        quint16 ch1 = readShort(&buf);
        quint16 ch2 = readShort(&buf);
        quint16 ch3 = readShort(&buf);
        quint16 ch4 = readShort(&buf);

        bool skip = false;
        if (colorSpace == 0) { // RGB
            e.color = KoColor(KoColorSpaceRegistry::instance()->rgb16());
            reinterpret_cast<quint16*>(e.color.data())[0] = ch3;
            reinterpret_cast<quint16*>(e.color.data())[1] = ch2;
            reinterpret_cast<quint16*>(e.color.data())[2] = ch1;
            e.color.setOpacity(OPACITY_OPAQUE_U8);
        }
        else if (colorSpace == 1) { // HSB
            e.color = KoColor(KoColorSpaceRegistry::instance()->rgb16());
            QColor c;
            c.setHsvF(ch1 / 65536.0, ch2 / 65536.0, ch3 / 65536.0);
            e.color.fromQColor(c);
            e.color.setOpacity(OPACITY_OPAQUE_U8);
        }
        else if (colorSpace == 2) { // CMYK
            e.color = KoColor(KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id(), ""));
            reinterpret_cast<quint16*>(e.color.data())[0] = quint16_MAX - ch1;
            reinterpret_cast<quint16*>(e.color.data())[1] = quint16_MAX - ch2;
            reinterpret_cast<quint16*>(e.color.data())[2] = quint16_MAX - ch3;
            reinterpret_cast<quint16*>(e.color.data())[3] = quint16_MAX - ch4;
            e.color.setOpacity(OPACITY_OPAQUE_U8);
        }
        else if (colorSpace == 7) { // LAB
            e.color = KoColor(KoColorSpaceRegistry::instance()->lab16());
            reinterpret_cast<quint16*>(e.color.data())[0] = ch3;
            reinterpret_cast<quint16*>(e.color.data())[1] = ch2;
            reinterpret_cast<quint16*>(e.color.data())[2] = ch1;
            e.color.setOpacity(OPACITY_OPAQUE_U8);
        }
        else if (colorSpace == 8) { // GRAY
            e.color = KoColor(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), ""));
            reinterpret_cast<quint16*>(e.color.data())[0] = ch1 * (quint16_MAX / 10000);
            e.color.setOpacity(OPACITY_OPAQUE_U8);
        }
        else {
            kWarning() << "Unsupported colorspace in palette" << filename() << "(" << colorSpace << ")";
            skip = true;
        }
        if (version == 2) {
            quint16 v2 = readShort(&buf);
            if (v2 != 2) {
                kWarning() << "Version 2 block is not version 2" << filename() << "(" << v2 << ")";
                return false;
            }
            quint16 size = readShort(&buf);
            QByteArray ba = buf.read(size);
            if (ba.size() != size) {
                kWarning() << "Version 2 name block is the wrong size" << filename();
                return false;
            }
            e.name = QString::fromUtf8(ba.constData(), ba.size());
        }
        if (!skip) {
            add(e);
        }
    }
    return true;
}

