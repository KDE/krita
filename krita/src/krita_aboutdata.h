/***************************************************************************
 *   Copyright (C) 2004 by Boudewijn Rempt                                 *
 *   boud@valdyas.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#ifndef krita_ABOUTDATA
#define krita_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>

static const char description[] = I18N_NOOP("krita KOffice Program");
static const char version[]     = "0.1";

KAboutData * newkritaAboutData()
{
    KAboutData * aboutData=new KAboutData( "krita", I18N_NOOP("krita"),
                                           version, description, KAboutData::License_GPL,
                                           "(c) 2001, Boudewijn Rempt");
    aboutData->addAuthor("Boudewijn Rempt",0, "boud@valdyas.org");
    return aboutData;
}

#endif
