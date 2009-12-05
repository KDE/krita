/*  This file is part of the KDE project
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>


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
#include "KoColorSet.h"

#include <QImage>
#include <QPoint>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceRegistry.h"

namespace {
    enum enumPaletteType {
        FORMAT_UNKNOWN,
        FORMAT_GPL, // Gimp palette
        FORMAT_PAL, // RIFF palette
        FORMAT_ACT // Photoshop binary color palette
    };

}


/*
KoColorSet::KoColorSet(const KisGradient * gradient, qint32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(gradient != 0);

    qreal dx, cur_x;
    qint32 i;
    dx = 1.0 / (nColors - 1);

    KoColorSetEntry e;
    KoColor c;
    for (i = 0, cur_x = 0; i < nColors; i++, cur_x += dx) {
        gradient->colorAt(c,cur_x);
        e.color = c;
        e.name = "Untitled";
        add(e);
    }

    m_columns = 0; // Set the default value that the GIMP uses...
}
*/
KoColorSet::KoColorSet(const QString& filename)
    : super(filename)
{
    // Implemented in super class
    m_columns = 0; // Set the default value that the GIMP uses...
}

KoColorSet::KoColorSet()
    : super("")
{
    m_columns = 0; // Set the default value that the GIMP uses...
}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : super("")
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
    file.open(QIODevice::ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}


bool KoColorSet::save()
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
        const KoColorSetEntry& entry = m_colors.at(i);
        QColor c = entry.color.toQColor();
        stream << c.red() << " " << c.green() << " " << c.blue() << "\t";
        if (entry.name.isEmpty())
            stream << "Untitled\n";
        else
            stream << entry.name << "\n";
    }

    file.close();
    return true;
}

qint32 KoColorSet::nColors()
{
    return m_colors.count();
}

bool KoColorSet::init()
{
    enumPaletteType format = FORMAT_UNKNOWN;

    QString s = QString::fromUtf8(m_data.data(), m_data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
        kWarning(30009) << "Illegal Gimp palette file: " << filename();
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

        QStringList lines = s.split('\n', QString::SkipEmptyParts);

        if (lines.size() < 3) {
            return false;
        }

        QString entry, channel, columns;
        QStringList c;
        qint32 r, g, b;
        QColor color;
        KoColorSetEntry e;

        format = FORMAT_GPL;

        // Read name
        if (!lines[1].startsWith("Name: ") || !lines[0].startsWith("GIMP") )
        {
            kWarning(30009) << "Illegal Gimp palette file: " << filename();
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
        for (qint32 i = index; i < lines.size(); i++) {
            if (lines[i].startsWith('#')) {
                m_comment += lines[i].mid(1).trimmed() + ' ';
            }
            else if (!lines[i].isEmpty())
            {
                QStringList a = lines[i].replace(QChar('\t'), " ").split(' ', QString::SkipEmptyParts);

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

                e.color = KoColor(KoColorSpaceRegistry::instance()->rgb8());
                e.color.fromQColor(QColor(r, g, b));

                QString name = a.join(" ");
                e.name = name.isEmpty() ? i18n("Untitled") : name;

                add(e);
            }
        }


        setValid(true);
        return true;
    }
    else if (s.length() == 768) {
        kWarning(30009) << "Photoshop format palette file. Not implemented yet";
        format = FORMAT_ACT;
    }
    return false;
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

KoColorSetEntry KoColorSet::getColor(quint32 index)
{
    return m_colors[index];
}
