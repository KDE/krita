/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include "kis_exif_info.h"

#include <stdlib.h>

#include <kdebug.h>

KisExifInfo::KisExifInfo()
{}


KisExifInfo::~KisExifInfo()
{}


bool KisExifInfo::load(const QDomElement& elmt)
{
    if(elmt.tagName() != "ExifInfo")
        return false;
    for( QDomNode node = elmt.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement e = node.toElement();
        if ( !e.isNull() )
        {
            if(e.tagName() == "ExifValue")
            {
                QString key = e.attribute("name");
                ExifValue eV;
                eV.load(e);
                setValue(key, eV);
            }
        }
    }
    return true;
}

QDomElement KisExifInfo::save(QDomDocument& doc)
{
    QDomElement elmt = doc.createElement("ExifInfo");
    for( KisExifInfo::evMap::const_iterator it = begin(); it != end(); ++it)
    {
        ExifValue ev = it.data();
        QDomElement evD = ev.save( doc);
        evD.setAttribute("name", it.key());
        elmt.appendChild(evD);
    }
    return elmt;
}
