/*
 *  kis_brush.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfileinfo.h>

#include <kimageeffect.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "kis_brush.h"
#include "kis_util.h"

#define THUMB_SIZE 30

KisBrush::KisBrush(QString file, bool monochrome, bool special) : super()
{
    // set defaults
    m_valid     = false;
    validThumb  = false;
    validPixmap = false;

    m_spacing = 7;
    m_hotSpot = QPoint( 0, 0 );

    // load the brush image data
    loadViaQImage(file, monochrome);

    if(m_valid)
    {
        int meanSize = (width() + height())/2;

        m_spacing  = meanSize / 4;
        if(m_spacing < 1)  m_spacing = 1;
        if(m_spacing > 20) m_spacing = 20;

        // default hotspot
        if(!special)
            m_hotSpot = QPoint(width()/2, height()/2);
        else
            m_hotSpot = QPoint(0, 0);

        // search and load the brushinfo file
        if(!special)
        {
            QFileInfo fi(file);
            file = fi.dirPath() + "/" + fi.baseName() + ".brushinfo";
            fi.setFile(file);
            if (fi.exists() && fi.isFile())
                readBrushInfo(file);
        }
    }
}


KisBrush::~KisBrush()
{
    if(hasValidPixmap())
    {
        delete[] m_pData;
        delete m_pPixmap;
    }

    if(hasValidThumb())
    {
        delete m_pThumbPixmap;
    }
}


void KisBrush::readBrushInfo(const QString& file)
{
    KSimpleConfig config(file, true);

    config.setGroup("General");
    int spacing = config.readNumEntry("Spacing", m_spacing);
    int hotspotX = config.readNumEntry("hotspotX", m_hotSpot.x());
    int hotspotY = config.readNumEntry("hotspotY", m_hotSpot.y());

    if(spacing > 0) m_spacing = spacing;
    if(hotspotX > 0 && hotspotY > 0) m_hotSpot = QPoint(hotspotX, hotspotY);
}


/*
    Load from file, actually
*/

void KisBrush::loadViaQImage(const QString& file, bool monochrome)
{
    QImage img(file);

    if (img.isNull())
    {
        m_valid = false;
        kdDebug()<<"Failed to load brush: "<< file.latin1()<<endl;
    }

    // scale a pixmap for iconview cell to size of cell
    if(img.width() > THUMB_SIZE || img.height() > THUMB_SIZE)
    {
        QPixmap filePixmap;
        filePixmap.load(file);
        QImage fileImage = filePixmap.convertToImage();

        int xsize = THUMB_SIZE;
        int ysize = THUMB_SIZE;
        int picW  = fileImage.width();
        int picH  = fileImage.height();

        if(picW > picH)
        {
            float yFactor = (float)((float)(float)picH/(float)picW);
            ysize = (int)(yFactor * (float)THUMB_SIZE);
            //kdDebug() << "ysize is " << ysize << endl;
            if (ysize > THUMB_SIZE) 
		    ysize = THUMB_SIZE;
        }
        else if(picW < picH)
        {
            float xFactor = (float)((float)picW/(float)picH);
            xsize = (int)(xFactor * (float)THUMB_SIZE);
            //kdDebug() << "xsize is " << xsize << endl;
            if(xsize > THUMB_SIZE) 
		    xsize = THUMB_SIZE;
        }

        QImage thumbImg = fileImage.smoothScale(xsize, ysize);

	if (!thumbImg.isNull()) {
		m_pThumbPixmap = new QPixmap();
		m_pThumbPixmap -> convertFromImage(thumbImg);
		validThumb = !m_pThumbPixmap -> isNull();
		
		if(!validThumb) {
			delete m_pThumbPixmap;
			m_pThumbPixmap = 0;
		}
	}
    }

    img = img.convertDepth(32);
    if(monochrome) img = KImageEffect::toGray(img, true);

    // create pixmap for preview
    m_pPixmap = new QPixmap;
    m_pPixmap->convertFromImage(img, QPixmap::AutoColor);

    m_w = img.width();
    m_h = img.height();

    m_pData = new uchar[m_h * m_w];
    uint *p;

    for (int h = 0; h < m_h; h++)
    {
        p = (QRgb*)img.scanLine(h);

        for (int w = 0; w < m_w; w++)
	    {
	        // no need to use qGray here, we have
            // converted the image to grayscale already
            if(monochrome)
	            m_pData[m_w * h + w] = 255 - qRed(*(p+w));
            else
	            m_pData[m_w * h + w] = *(p+w);
	    }
    }

    m_valid = true;
    validPixmap = true;

    // kdDebug()<<"Loading brush: "<<file.latin1()<<endl;
}



void KisBrush::setHotSpot(QPoint pt)
{
    int x = pt.x();
    int y = pt.y();

    if (x < 0) x = 0;
    else if (x >= m_w) x = m_w-1;

    if (y < 0) y = 0;
    else if (y >= m_h) y = m_h-1;

    m_hotSpot = QPoint(x,y);
}


uchar KisBrush::value(int x, int y) const
{
    return m_pData[m_w * y + x];
}


uchar* KisBrush::scanline(int i) const
{
    if (i < 0) i = 0;
    if (i >= m_h) i = m_h-1;

    return (m_pData + m_w * i);
}

uchar* KisBrush::bits() const
{
    return m_pData;
}

void KisBrush::dump() const
{
    kdDebug()<<"KisBrush data:\n";

    for (int h = 0; h < m_h; h++)
    {
        for (int w = 0; w < m_w; w++)
        {
            kdDebug()<<" :"<< m_pData[m_w * h + w]<<endl;
        }
    }
}

