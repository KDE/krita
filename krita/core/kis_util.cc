/*
 *  kis_util.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *                1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include <list>

#include <Magick++.h>

#include <kdebug.h>
#include <kimageio.h>
#include <kglobal.h>
#include <klocale.h>

#include "kis_global.h"
#include "kis_util.h"

using namespace std;
using namespace Magick;

// A number which can be added to any image coordinate to make it positive
// Used to make numbers round towards + or - infinity regardless of sign
const long BIGNUM = (TILE_SIZE*10000);


void KisUtil::printRect( const QRect& r, const QString& name )
{
    kdDebug()<<name.latin1()<<":: l:"<<r.left()<<" t:"<< r.top()<<" r:"<<r.right()<<" b:"<<r.bottom()<<" w:"<<r.width()<<" h: "<< r.height()<<endl;
}

void KisUtil::printPoint( const QPoint& p, const QString& name )
{
    kdDebug()<< name.latin1() <<" :: x:"<<p.x()<<" y:"<<p.y()<<endl;
}

void KisUtil::enlargeRectToContainPoint( QRect& r, QPoint p )
{
  if (r.contains(p))
    {
        kdDebug()<<"KisUtil::enlargeRectToContainPoint: point already contained\n";
      return;
    }
  if (p.x()<r.left())   r.setLeft(p.x());
  if (p.x()>r.right())  r.setRight(p.x());
  if (p.y()<r.top())    r.setTop(p.y());
  if (p.y()>r.bottom()) r.setBottom(p.y());
}

// Find a rectangle which encloses r whose coordinates are divisible
// by TILE_SIZE (ie no remainder)
QRect KisUtil::findTileExtents( QRect r )
{
  r.setLeft(((r.left()+BIGNUM)/TILE_SIZE)*TILE_SIZE-BIGNUM);
  r.setTop(((r.top()+BIGNUM)  /TILE_SIZE)*TILE_SIZE-BIGNUM);
  r.setBottom(((r.bottom()+TILE_SIZE+BIGNUM)/TILE_SIZE)*TILE_SIZE-BIGNUM-1);
  r.setRight(((r.right()+TILE_SIZE+BIGNUM)  /TILE_SIZE)*TILE_SIZE-BIGNUM-1);
  return(r);
}

#if 0
QString KisUtil::channelIdtoString(cId cid)
{
  switch (cid)
	{
	case ci_Indexed:
	  return i18n("indexed");
	  break;
	case ci_Alpha:
	  return ("alpha");
	  break;
	case ci_Red:
	  return i18n("red");
	  break;
	case ci_Green:
	  return i18n("green");
	  break;
	case ci_Blue:
	  return i18n("blue");
	  break;
	case ci_Cyan:
	  return i18n("cyan");
	  break;
	case ci_Magenta:
	  return i18n("magenta");
	  break;
	case ci_Yellow:
	  return i18n("yellow");
	  break;
	case ci_Black:
	  return i18n("black");
	  break;
	case ci_L:
	  return "L";
	  break;
	case ci_a:
	  return "a";
	  break;
	case ci_b:
	  return "b";
	  break;
	default:
	  return i18n("unknown color", "unknown");
	  break;
	}
}
#endif

/**
 * @name readFilters
 * @return Provide a list of file formats the application can read.
 */
QString KisUtil::readFilters()
{
	typedef list<CoderInfo> ci;
	typedef list<CoderInfo>::iterator ci_it;

	list<CoderInfo> coders;
	QString s = " ";
	QString name;
	QString description;

	coderInfoList(&coders, CoderInfo::TrueMatch, CoderInfo::AnyMatch, CoderInfo::AnyMatch);

	for (ci_it it = coders.begin(); it != coders.end(); it++) {
		name = (*it).name().c_str();
		s += "*." + name.lower() + " *." + name + " ";
	}

	s += "|" + i18n("All pictures");
	s += "\n";

	for (ci_it it = coders.begin(); it != coders.end(); it++) {
		name = (*it).name().c_str();
		description = (*it).description().c_str();

		if (!description.contains('/')) {
			s += "*." + name.lower() + " *." + name + "|";
			s += i18n(description.latin1());
			s += "\n";
		}
	}

	return s;
}

QString KisUtil::writeFilters()
{
	return KisUtil::readFilters();
}

/*
    roughScaleQImage - scale a qimage keeping palette, no anti-aliasing
    or color blending based on surrounding pixel values

    based on:
        libfbx-stretch.c -- Surface Stretching Functions
        (C)opyright 2000 U4X Labs - LGPL licensing
        Written by: Mike Bourgeous <nitrogen@u4x.org>
*/

QImage KisUtil::roughScaleQImage(QImage & src, int width, int height)
{
    QImage dest(width, height, src.depth());

    int xpos, ypos;
    int x1, y1;
    int x = 0, y = 0;

    float ratio_x = (float)src.width() / (float)width;
    float ratio_y = (float)src.height() / (float)height;

    for (ypos = y; ypos < y + height; ypos++)
    {
        for (xpos = x; xpos < x + width; xpos++)
        {
            x1 = (int)((xpos - x) * ratio_x);
            y1 = (int)((ypos - y) * ratio_y);

            QRgb value = src.pixel(x1, y1);
            dest.setPixel(xpos, ypos, value);
        }
    }

    return dest;
}

QRect KisUtil::findBoundingTiles(const QRect& area)
{
	QRect rc;

	rc.setLeft(area.left() / TILE_SIZE);
	rc.setTop(area.top() / TILE_SIZE);
	rc.setRight(area.right() / TILE_SIZE);
	rc.setBottom(area.bottom() / TILE_SIZE);

	if (rc.left() < 0)
		rc.setLeft(0);

	if (rc.top() < 0)
		rc.setTop(0);
	
	return rc;
}
