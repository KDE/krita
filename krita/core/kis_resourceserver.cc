/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_factory.h"
#include "kis_resourceserver.h"
#include "kis_timer.h"

KisResourceServer::KisResourceServer()
{

KisTimer::start();

    m_brushes.setAutoDelete(true);
    m_patterns.setAutoDelete(true);

    // image formats
    QStringList formats;
    formats << "*.png" << "*.tif" << "*.xpm" << "*.bmp" << "*.jpg" << "*.gif";

    // init vars
    QStringList  lst;
    QString format, file;

    // find brushes
    for ( QStringList::Iterator it = formats.begin();
    it != formats.end(); ++it )
    {
      format = *it;
	  QStringList l = KisFactory::global()->dirs()->findAllResources("kis_brushes", format, false, true);
	  lst += l;
	}
    // load brushes
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
    {
      file = *it;
      (void) loadBrush( file );
    }

    // reset vars
    lst.clear();
    format = file = "";

    // find pattern
    for ( QStringList::Iterator it = formats.begin(); it != formats.end(); ++it )
    {
      format = *it;
	  QStringList l = KisFactory::global()->dirs()->findAllResources("kis_pattern", format, false, true);
	  lst += l;
	}
    // load pattern
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
    {
      file = *it;
      (void) loadPattern( file );
    }

KisTimer::stop("KisResourceServer()");

}

KisResourceServer::~KisResourceServer()
{
    m_brushes.clear();
    m_patterns.clear();
}

const KisBrush * KisResourceServer::loadBrush( const QString& filename )
{
    KisBrush *brush = new KisBrush( filename, false, false );

    if ( brush->isValid() )
        m_brushes.append(brush);
    else
    {
        delete brush;
        brush = 0L;
    }

    return brush;
}

const KisPattern * KisResourceServer::loadPattern( const QString& filename )
{
    KisPattern *pattern = new KisPattern( filename );

    if ( pattern->isValid() )
        m_patterns.append(pattern);
    else
    {
        delete pattern;
        pattern = 0L;
    }

    return pattern;
}

