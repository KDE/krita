/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <netinet/in.h>
#include <qimage.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qvaluevector.h>
#include <qsize.h>

#include <kdebug.h>
#include <kimageeffect.h>
#include <ksimpleconfig.h>

#include "kis_brush.h"

#define SPACING_DEFAULT 7

namespace {
	struct GimpBrushHeader {
		Q_UINT32 header_size;  /*  header_size = sizeof (BrushHeader) + brush name  */
		Q_UINT32 version;      /*  brush file version #  */
		Q_UINT32 width;        /*  width of brush  */
		Q_UINT32 height;       /*  height of brush  */
		Q_UINT32 bytes;        /*  depth of brush in bytes--always 1 */
		Q_UINT32 magic_number; /*  GIMP brush magic number  */
		Q_UINT32 spacing;      /*  brush spacing  */
	};
}

KisBrush::KisBrush(const QString& filename) : super(filename)
{
	m_spacing = SPACING_DEFAULT;
	m_hotSpot = QPoint(0, 0);

	// load the brush image data
//	loadViaQImage(filename, monochrome);

#if 0
	if (m_valid) {
		Q_INT32 meanSize = (width() + height()) / 2;

		m_spacing = meanSize / 4;

		if (m_spacing < 1)  
			m_spacing = 1;

		if (m_spacing > 20) 
			m_spacing = 20;

		// default hotspot
		if (special)
			m_hotSpot = QPoint(0, 0);
		else
			m_hotSpot = QPoint(width() / 2, height() / 2);

		// search and load the brushinfo file
		if (!special) {
			QFileInfo fi(filename);
			QString filename = fi.dirPath() + "/" + fi.baseName() + ".brushinfo";

			fi.setFile(filename);
//			if (fi.exists() && fi.isFile())
//				readBrushInfo(filename);
		}
	}
#endif
}

KisBrush::~KisBrush()
{
}

bool KisBrush::loadAsync()
{
	// TODO: This is really loadSync, actually implement loadAsync
	QFile file(filename());
	Q_ULONG nbytes;
	GimpBrushHeader bh;
	QValueVector<char> vname;
	QString brName;

	if (!file.open(IO_ReadOnly))
		return false;

	nbytes = sizeof(GimpBrushHeader);

	if ((nbytes = file.readBlock(reinterpret_cast<char*>(&bh), nbytes)) < sizeof(GimpBrushHeader))
		return false;

	bh.header_size = ntohl(bh.header_size);
	bh.version = ntohl(bh.version);
	bh.width = ntohl(bh.width);
	bh.height = ntohl(bh.height);
	bh.bytes = ntohl(bh.bytes);
	bh.magic_number = ntohl(bh.magic_number);
	bh.spacing = ntohl(bh.spacing);
	vname.resize(bh.header_size - sizeof(GimpBrushHeader));

	if (file.readBlock(&vname[0], vname.size()) != static_cast<Q_LONG>(vname.size()))
		return false;

	brName = &vname[0];
	setName(brName);
	nbytes = bh.width * bh.height * bh.bytes;
	vname.resize(nbytes);

	if (file.readBlock(&vname[0], vname.size()) != static_cast<Q_LONG>(vname.size()))
		return false;

	if (!m_img.create(bh.width, bh.height, 32))
		return false;

	if (bh.bytes == 1) {
		for (Q_UINT32 y = 0, k = 0; y < bh.height; y++) {
			for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
				Q_INT32 val = 255 - vname[k];

				m_img.setPixel(x, y, qRgb(val, val, val));
			}
		}
	} else if (bh.bytes == 4) {
		for (Q_UINT32 y = 0, k = 0; y < bh.height; y++)
			for (Q_UINT32 x = 0; x < bh.width; x++)
				m_img.setPixel(x, y, qRgba(255 - vname[k++], 255 - vname[k++], 255 - vname[k++], 255 - vname[k++]));
	} else {
		return false;
	}

	setWidth(m_img.width());
	setHeight(m_img.height());
	setValid(true);
	emit loadComplete(this);
	return true;
}

bool KisBrush::saveAsync()
{
	return false;
}

QImage KisBrush::img() const
{
	return m_img;
}

QImage KisBrush::frame(Q_INT32) const
{
	return m_img;
}

void KisBrush::setHotSpot(QPoint pt)
{
	Q_INT32 x = pt.x();
	Q_INT32 y = pt.y();

	if (x < 0) 
		x = 0;
	else if (x >= width()) 
		x = width() - 1;

	if (y < 0) 
		y = 0;
	else if (y >= height()) 
		y = height() - 1;

	m_hotSpot = QPoint(x,y);
}


uchar KisBrush::value(Q_INT32 x, Q_INT32 y) const
{
	return m_pData[width() * y + x];
}

uchar *KisBrush::scanline(Q_INT32 i) const
{
	if (i < 0) 
		i = 0;

	if (i >= height()) 
		i = height() - 1;

	return (m_pData + width() * i);
}

uchar *KisBrush::bits() const
{
	if (valid())
		return m_img.bits();

	return 0;
}

#include "kis_brush.moc"
