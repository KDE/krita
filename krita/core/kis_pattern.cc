/*
 *  kis_pattern.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
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

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfileinfo.h>

#include <kimageeffect.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "kis_pattern.h"

#define THUMB_SIZE 30


KisPattern::KisPattern(QString file)
  : KisKrayon()
{
    m_valid    = false;
    m_spacing  = 4;
    m_hotSpot = QPoint(0, 0);

    loadViaQImage(file);

    if(m_valid)
    {
        int meanSize = (width() + height())/2;

        m_spacing  = meanSize / 4;
        if(m_spacing < 1)  m_spacing = 1;
        if(m_spacing > 20) m_spacing = 20;

        // default hotspot
        m_hotSpot = QPoint(width()/2, height()/2);

        // search and load the brushinfo file
        QFileInfo fi(file);
        file = fi.dirPath() + "/" + fi.baseName() + ".patterninfo";
        fi.setFile(file);
        if (fi.exists() && fi.isFile())
            readPatternInfo(file);
    }
}


KisPattern::KisPattern(int formula)
  : KisKrayon()
{
    m_valid = false;
    validThumb = false;
    m_spacing  = 3;
    loadViaFormula(formula);
}


KisPattern::~KisPattern()
{
    delete m_pImage;
    delete m_pPixmap;
}


void KisPattern::loadViaQImage(QString file)
{
    // load via QImage
    m_pImage = new QImage(file);
    m_pImage->setAlphaBuffer(true);

    if (m_pImage->isNull())
    {
        m_valid = false;
        kdDebug()<<"Failed to load pattern: "<< file.latin1()<<endl;
    }

    *m_pImage = m_pImage->convertDepth(32);

    // create pixmap for preview dialog
    m_pPixmap = new QPixmap;
    QImage img = *m_pImage;
    m_pPixmap->convertFromImage(img, QPixmap::AutoColor);

    // scale a pixmap for iconview cell to size of cell
    if(img.width() > THUMB_SIZE || img.height() > THUMB_SIZE)
    {
        QPixmap filePixmap;
        filePixmap.load(file);
        QImage fileImage = filePixmap.convertToImage();

        m_pThumbPixmap = new QPixmap;

        int xsize = THUMB_SIZE;
        int ysize = THUMB_SIZE;
        int picW  = fileImage.width();
        int picH  = fileImage.height();

        if(picW > picH)
        {
            float yFactor = (float)((float)(float)picH/(float)picW);
            ysize = (int)(yFactor * (float)THUMB_SIZE);
            //kdDebug() << "ysize is " << ysize << endl;
            if(ysize > 30) ysize = 30;
        }
        else if(picW < picH)
        {
            float xFactor = (float)((float)picW/(float)picH);
            xsize = (int)(xFactor * (float)THUMB_SIZE);
            //kdDebug() << "xsize is " << xsize << endl;
            if(xsize > 30) xsize = 30;
        }

        QImage thumbImg = fileImage.smoothScale(xsize, ysize);

        if(!thumbImg.isNull())
        {
            m_pThumbPixmap->convertFromImage(thumbImg);
            if(!m_pThumbPixmap->isNull())
            {
                validThumb = true;
            }
        }
    }

    m_w = m_pImage->width();
    m_h = m_pImage->height();

    m_valid = true;
    //kdDebug()<<"Loading pattern: "<<file.latin1()<<endl;
}

/*
    load pattern from a formula or algorithm - these will
    algorithms and/or predefined Qt patterns

    Formulas for patterns should come from plugins which
    could be written in almost any language to generate
    patterns in a given area or region
*/
void KisPattern::loadViaFormula(int formula)
{
    // load via QImage
    m_pImage = new QImage(THUMB_SIZE, THUMB_SIZE, 32);

    if (m_pImage->isNull())
    {
        m_valid = false;
        kdDebug()<<"Failed to load pattern: "<< formula<<endl;
    }

    // create pixmap for preview dialog
    m_pPixmap = new QPixmap;
    m_pPixmap->convertFromImage(*m_pImage, QPixmap::AutoColor);

    switch(formula)
    {
        default:
            m_pPixmap->fill(Qt::white);
            break;
    }

    m_w = m_pImage->width();
    m_h = m_pImage->height();

    m_valid = true;
    //kdDebug()<<"Loading pattern: "<< formula<<endl;
}

void KisPattern::readPatternInfo(QString file)
{
    KSimpleConfig config(file, true);

    config.setGroup("General");
    int spacing = config.readNumEntry("Spacing", m_spacing);
    int hotspotX = config.readNumEntry("hotspotX", m_hotSpot.x());
    int hotspotY = config.readNumEntry("hotspotY", m_hotSpot.y());

    if(spacing > 0) m_spacing = spacing;
    if(hotspotX > 0 && hotspotY > 0) m_hotSpot = QPoint(hotspotX, hotspotY);
}

