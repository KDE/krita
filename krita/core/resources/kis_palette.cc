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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include <kdebug.h>
#include <klocale.h>

#include "kis_global.h"
#include "kis_palette.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"


namespace {
	enum enumPaletteType {
		FORMAT_UNKNOWN,
		FORMAT_GPL, // Gimp palette
		FORMAT_PAL, // RIFF palette
		FORMAT_ACT // Photoshop binary color palette
	};

};


KisPalette::KisPalette(const QImage * img, Q_INT32 nColors, const QString & name)
	: super(QString("")),
	  m_name(name)
{
	Q_ASSERT(nColors > 0);
	Q_ASSERT(!img -> isNull());

	// XXX: Implement

}

KisPalette::KisPalette(const KisPaintDeviceSP device, Q_INT32 nColors, const QString & name)
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

bool KisPalette::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisPalette::saveAsync()
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

void KisPalette::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisPalette::ioResult(KIO::Job * /*job*/)
{
	enumPaletteType format = FORMAT_UNKNOWN;

	QString s = QString::fromUtf8(m_data.data(), m_data.count());

	if (s.isEmpty() || s.isNull() || s.length() < 50) {
// 		kdDebug() << "Illegal Gimp palette file: " << filename() << "\n";
                setValid(false);
                emit loadComplete(this);
                return;
        }


	if (s.startsWith("RIFF") || s.startsWith("PAL data"))
	{
//		kdDebug() << "PAL format palette file\n";
		format = FORMAT_PAL;
	}
	else if (s.startsWith("GIMP Palette"))
	{
		// XXX: No checks for wrong input yet!
		Q_UINT32 index = 0;

		QStringList lines = QStringList::split("\n", s);

		QString entry, channel, columns;
		QStringList c;
		Q_INT32 r, g, b;
		QColor color;
		KisPaletteEntry e;

//		kdDebug() << "Gimp format palette file\n";
		format = FORMAT_GPL;

		// Read name
		if (!lines[1].startsWith("Name: ") || !lines[0].startsWith("GIMP") )
		{
// 			kdDebug() << "Illegal Gimp palette file: " << filename() << "\n";
			setValid(false);
			emit loadComplete(this);
			return;
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
// 			kdDebug() << "line: " << lines[i] << "\n";
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
		emit loadComplete(this);

		return;
	}
	else if (s.length() == 768) {
// 		kdDebug() << "Photoshop format palette file. Not implemented yet\n";
		format = FORMAT_ACT;
	}
	
	setValid(false);
	emit loadComplete(this);


}


void KisPalette::add(const KisPaletteEntry & c)
{
// 	kdDebug() << "Added: " << c.color.red() << ", " 
// 		  << c.color.green() << ", " 
// 		  << c.color.blue() << ", " 
// 		  << c.name << "\n";
	m_colors.push_back(c);
}

void KisPalette::remove(const KisPaletteEntry & c)
{
	Q_UNUSED(c);
// 	QValueVector<KisPaletteEntry>::iterator it = qFind(m_colors.begin(), m_colors.end(), c);
// 	if (it != m_colors.end()) {
// 		m_colors.erase(it);
// 	}
}

KisPaletteEntry KisPalette::getColor(Q_UINT32 index)
{
	return m_colors[index];
}
