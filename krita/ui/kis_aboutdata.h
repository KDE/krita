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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ABOUT_DATA_H_
#define KIS_ABOUT_DATA_H_

#include <kaboutdata.h>
#include <klocale.h>
#include <kofficeversion.h>
#include <config.h>

KAboutData * newKritaAboutData()
{
    KAboutData * aboutData = new KAboutData( "krita",
            I18N_NOOP("Krita"),
            KOFFICE_VERSION_STRING,
            I18N_NOOP("KOffice image manipulation application"),
            KAboutData::License_GPL,
            I18N_NOOP("(c) 1999-2006 The Krita team.\n"),
            "",
            "http://www.koffice.org/krita",
            "submit@bugs.kde.org");
    aboutData->addAuthor("Adrian Page", 0, "Adrian.Page@tesco.net");
    aboutData->addAuthor("Alan Horkan", 0, "", "http://www.openclipart.org");
    aboutData->addAuthor("Bart Coppens", 0, "kde@bartcoppens.be");
    aboutData->addAuthor("Boudewijn Rempt", 0, "boud@valdyas.org", "http://www.valdyas.org/fading/index.cgi");
    aboutData->addAuthor("Carsten Pfeiffer", 0, "carpdjih@cetus.zrz.tu-berlin.de");
    aboutData->addAuthor("Casper Boemann", 0, "cbr@boemann.dk");
    aboutData->addAuthor("Clarence Dang", 0, "dang@kde.org");
    aboutData->addAuthor("Cyrille Berger", 0, "cyb@lepi.org");
    aboutData->addAuthor("Dirk Schoenberger", 0, "dirk.schoenberger@sz-online.de");
    aboutData->addAuthor("Danny Allen", 0 , "danny@dannyallen.co.uk");
    aboutData->addAuthor("Emanuele Tamponi", 0, "emanuele@valinor.it");
    aboutData->addAuthor("Gábor Lehel", 0, "<illissius@gmail.com>");
    aboutData->addAuthor("John Califf",0, "jcaliff@compuzone.net");
    aboutData->addAuthor("Laurent Montel",0, "lmontel@mandrakesoft.com");
    aboutData->addAuthor("Matthias Elter", 0, "me@kde.org");
    aboutData->addAuthor("Melchior Franz", 0, "mfranz@kde.org");
    aboutData->addAuthor("Michael Koch", 0, "koch@kde.org");
    aboutData->addAuthor("Michael Thaler", 0, "michael.thaler@physik.tu-muenchen.de");
    aboutData->addAuthor("Patrick Julien", 0, "freak@codepimps.org");
    aboutData->addAuthor("Roger Larsson", 0, "roger.larsson@norran.net");
    aboutData->addAuthor("Sven Langkamp", 0, "longamp@reallygood.de");
    aboutData->addAuthor("Toshitaka Fujioka", 0, "fujioka@kde.org");
    aboutData->addAuthor("Thomas Zander", 0, "zander@kde.org");
    aboutData->addAuthor("Sander Koning", 0, "sanderkoning@kde.nl");
    aboutData->addAuthor("Ronan Zeegers",/*"icons"*/ 0,0);
    return aboutData;
}

#endif // KIS_ABOUT_DATA_H_
