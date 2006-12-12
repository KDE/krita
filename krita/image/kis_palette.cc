/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <netinet/in.h>
#include <limits.h>
#include <stdlib.h>
#include <cfloat>

#include <QImage>
#include <QPoint>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include "kis_debug_areas.h"
#include "kis_palette.h"
#include "kis_iterators_pixel.h"


namespace {
    enum enumPaletteType {
        FORMAT_UNKNOWN,
        FORMAT_GPL, // Gimp palette
        FORMAT_PAL, // RIFF palette
        FORMAT_ACT // Photoshop binary color palette
    };

}


KisPalette::KisPalette(const QImage * img, qint32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(!img->isNull());

    // XXX: Implement

    m_columns = 0; // Set the default value that the GIMP uses...
}

KisPalette::KisPalette(const KisPaintDeviceSP device, qint32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(!device.isNull());


    // XXX: Implement
    m_columns = 0; // Set the default value that the GIMP uses...
}


KisPalette::KisPalette(const KisGradient * gradient, qint32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(gradient != 0);

    double dx, cur_x;
    qint32 i;
    dx = 1.0 / (nColors - 1);

    KisPaletteEntry e;
    KoColor c;
    for (i = 0, cur_x = 0; i < nColors; i++, cur_x += dx) {
        gradient->colorAt(c,cur_x);
        e.color = c.toQColor();
        e.name = "Untitled";
        add(e);
    }

    m_columns = 0; // Set the default value that the GIMP uses...
}

KisPalette::KisPalette(const QString& filename)
    : super(filename)
{
    // Implemented in super class
    m_columns = 0; // Set the default value that the GIMP uses...
}

KisPalette::KisPalette()
    : super("")
{
    m_columns = 0; // Set the default value that the GIMP uses...
}

/// Create an copied palette
KisPalette::KisPalette(const KisPalette& rhs)
    : super("")
{
    setFilename(rhs.filename());
    m_ownData = false;
    m_img = rhs.m_img;
    m_name = rhs.m_name;
    m_comment = rhs.m_comment;
    m_columns = rhs.m_columns;
    m_colors = rhs.m_colors;
    setValid(true);
}

KisPalette::~KisPalette()
{
}

bool KisPalette::load()
{
    QFile file(filename());
    file.open(QIODevice::ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}


bool KisPalette::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QTextStream stream(&file);
    // Header: Magic\nName: <name>\nColumns: <no idea what this means, but default = 0>
    // In any case, we don't use Columns...
    stream << "GIMP Palette\nName: " << name() << "\nColumns: " << m_columns << "\n#\n";

    for (int i = 0; i < m_colors.size(); i++) {
        const KisPaletteEntry& entry = m_colors.at(i);
        QColor c = entry.color;
        stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
        if (entry.name.isEmpty())
            stream << "Untitled\n";
        else
            stream << entry.name << "\n";
    }

    file.close();
    return true;
}

QImage KisPalette::img()
{
    return m_img;
}

qint32 KisPalette::nColors()
{
    return m_colors.count();
}

bool KisPalette::init()
{
    enumPaletteType format = FORMAT_UNKNOWN;

    QString s = QString::fromUtf8(m_data.data(), m_data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
        kWarning(DBG_AREA_FILE) << "Illegal Gimp palette file: " << filename() << "\n";
        return false;
    }


    if (s.startsWith("RIFF") || s.startsWith("PAL data"))
    {
       format = FORMAT_PAL;
    }
    else if (s.startsWith("GIMP Palette"))
    {
        // XXX: No checks for wrong input yet!
        quint32 index = 0;

        QStringList lines = s.split("\n", QString::SkipEmptyParts);

        if (lines.size() < 3) {
            return false;
        }

        QString entry, channel, columns;
        QStringList c;
        qint32 r, g, b;
        QColor color;
        KisPaletteEntry e;

        format = FORMAT_GPL;

        // Read name
        if (!lines[1].startsWith("Name: ") || !lines[0].startsWith("GIMP") )
        {
            kWarning(DBG_AREA_FILE) << "Illegal Gimp palette file: " << filename() << "\n";
            return false;
        }

        setName(i18n(lines[1].mid(strlen("Name: ")).trimmed().toAscii()));

        index = 2;

        // Read columns
        if (lines[index].startsWith("Columns: ")) {
            columns = lines[index].mid(strlen("Columns: ")).trimmed();;
            m_columns = columns.toInt();
            index = 3;
        }
        for (Q_INT32 i = index; i < lines.size(); i++) {
            if (lines[i].startsWith("#")) {
                m_comment += lines[i].mid(1).trimmed() + " ";
            }
            else if (!lines[i].isEmpty())
            {
                QStringList a = lines[i].replace(QChar('\t'), " ").split(" ");

                if (a.count() < 3)
                {
                    break;
                }

                r = a[0].toInt();
                a.pop_front();
                g = a[0].toInt();
                a.pop_front();
                b = a[0].toInt();
                a.pop_front();

                if (r < 0 || r > 255 ||
                    g < 0 || g > 255 ||
                    b < 0 || b > 255)
                {
                    break;
                }

                color = QColor(r, g, b);
                e.color = color;

                QString name = a.join(" ");
                e.name = name.isEmpty() ? i18n("Untitled") : name;

                add(e);
            }
        }


        setValid(true);
        return true;
    }
    else if (s.length() == 768) {
        kWarning(DBG_AREA_FILE) << "Photoshop format palette file. Not implemented yet\n";
        format = FORMAT_ACT;
    }
    return false;
}


void KisPalette::add(const KisPaletteEntry & c)
{
    m_colors.push_back(c);
}

void KisPalette::remove(const KisPaletteEntry & c)
{
    QVector<KisPaletteEntry>::iterator it = m_colors.begin();
    QVector<KisPaletteEntry>::iterator end = m_colors.end();

    while (it != end) {
        if ((*it) == c) {
            m_colors.erase(it);
            return;
        }
        ++it;
    }
}

KisPaletteEntry KisPalette::getColor(quint32 index)
{
    return m_colors[index];
}

#include "kis_palette.moc"
