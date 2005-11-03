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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <netinet/in.h>
#include <limits.h>
#include <stdlib.h>
#include <cfloat>

#include <qimage.h>
#include <qpoint.h>
#include <qvaluevector.h>
#include <qfile.h>

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


KisPalette::KisPalette(const QImage * img, Q_INT32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(!img -> isNull());

    // XXX: Implement

}

KisPalette::KisPalette(const KisPaintDeviceImplSP device, Q_INT32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(device != 0);


    // XXX: Implement
}


KisPalette::KisPalette(const KisGradient * gradient, Q_INT32 nColors, const QString & name)
    : super(QString("")),
      m_name(name)
{
    Q_ASSERT(nColors > 0);
    Q_ASSERT(gradient != 0);

    double dx, cur_x;
    QColor c;
    Q_INT32 i;
    Q_UINT8 opacity;
    dx = 1.0 / (nColors - 1);

    KisPaletteEntry e;
    for (i = 0, cur_x = 0; i < nColors; i++, cur_x += dx) {
        gradient -> colorAt(cur_x, &e.color, &opacity);
        e.name = "Untitled";
        add(e);
    }
}

KisPalette::KisPalette(const QString& filename)
    : super(filename)
{
    // Implemented in super class
}

 
KisPalette::~KisPalette()
{
}

bool KisPalette::load()
{
    QFile file(filename());
    file.open(IO_ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}


bool KisPalette::save()
{
    return false;
}

QImage KisPalette::img()
{
    return m_img;
}

Q_INT32 KisPalette::nColors()
{
    return m_colors.count();
}

bool KisPalette::init()
{
    enumPaletteType format = FORMAT_UNKNOWN;

    QString s = QString::fromUtf8(m_data.data(), m_data.count());

    if (s.isEmpty() || s.isNull() || s.length() < 50) {
         kdDebug(DBG_AREA_FILE) << "Illegal Gimp palette file: " << filename() << "\n";
                return false;
        }


    if (s.startsWith("RIFF") || s.startsWith("PAL data"))
    {
        kdDebug(DBG_AREA_FILE) << "PAL format palette file\n";
        format = FORMAT_PAL;
    }
    else if (s.startsWith("GIMP Palette"))
    {
        // XXX: No checks for wrong input yet!
        Q_UINT32 index = 0;

        QStringList lines = QStringList::split("\n", s);
    
        if (lines.size() < 3) {
            return false;
        }

        QString entry, channel, columns;
        QStringList c;
        Q_INT32 r, g, b;
        QColor color;
        KisPaletteEntry e;

        kdDebug(DBG_AREA_FILE) << "Gimp format palette file\n";
        format = FORMAT_GPL;

        // Read name
        if (!lines[1].startsWith("Name: ") || !lines[0].startsWith("GIMP") )
        {
             kdDebug(DBG_AREA_FILE) << "Illegal Gimp palette file: " << filename() << "\n";
            return false;
        }

        setName(i18n(lines[1].mid(strlen("Name: ")).stripWhiteSpace().ascii()));
        
        index = 2;

        // Read columns
        if (lines[index].startsWith("Columns: ")) {
            columns = lines[index].mid(strlen("Columns: ")).stripWhiteSpace();;
            m_columns = columns.toInt();
            index = 3;
        }

        // Loop over the rest of the lines
        for (Q_UINT32 i = index; i < lines.size() - 1; i++) {
            if (lines[i].startsWith("#")) {
                m_comment += lines[i].mid(1).stripWhiteSpace() + " ";
            }
            else {
                if (lines[i].contains("\t") > 0) {
                    QStringList a = QStringList::split("\t", lines[i]);
                    e.name = a[1];

                    QStringList c = QStringList::split(" ", a[0]);
                    channel = c[0].stripWhiteSpace();
                    r = channel.toInt();
                    channel = c[1].stripWhiteSpace();
                    g = channel.toInt();
                    channel = c[2].stripWhiteSpace();
                    b = channel.toInt();
                    color = QColor(r, g, b);
                    e.color = color;

                    add(e);
                }
            }
        }

        setValid(true);
        return true;
    }
    else if (s.length() == 768) {
         kdDebug(DBG_AREA_FILE) << "Photoshop format palette file. Not implemented yet\n";
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
    Q_UNUSED(c);
}

KisPaletteEntry KisPalette::getColor(Q_UINT32 index)
{
    return m_colors[index];
}

#include "kis_palette.moc"
