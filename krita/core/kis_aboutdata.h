/*
 *  kis_aboutdata.h - part of Krayon
 *
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
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

#ifndef KRAYON_ABOUTDATA
#define KRAYON_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>

KAboutData * newKrayonAboutData()
{
    KAboutData * aboutData = new KAboutData( "krayon",
                                             I18N_NOOP("Krayon"),
                                             "0.2.0",
                                             I18N_NOOP("KOffice image manipulation application."),
                                             KAboutData::License_GPL,
                                             "(c) 1999-2001 The Krayon team.",
                                             "",
                                             "http://koffice.kde.org/krayon",
                                             "submit@bugs.kde.org");
    aboutData->addAuthor("Matthias Elter", 0, "me@kde.org");
    aboutData->addAuthor("Carsten Pfeiffer", 0, "carpdjih@cetus.zrz.tu-berlin.de");
    aboutData->addAuthor("Michael Koch", 0, "koch@kde.org");
    aboutData->addAuthor("John Califf",0, "jcaliff@compuzone.net");
    aboutData->addAuthor("Laurent Montel",0, "lmontel@mandrakesoft.com");
    aboutData->addAuthor("Toshitaka Fujioka", 0, "fujioka@kde.org");
    return aboutData;
}

#endif
