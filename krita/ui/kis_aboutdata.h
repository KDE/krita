/*
 *  kis_aboutdata.h - part of Krayon
 *
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
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

KAboutData * newKritaAboutData()
{
    KAboutData * aboutData = new KAboutData("krita", 0,
                                            ki18n("Krita"),
                                            KOFFICE_VERSION_STRING,
                                            ki18n("KOffice image manipulation application"),
                                            KAboutData::License_GPL,
                                            ki18n("(c) 1999-2009 The Krita team.\n"),
                                            KLocalizedString(),
                                            "http://www.krita.org",
                                            "submit@bugs.kde.org");
    aboutData->addAuthor(ki18n("Adrian Page"), KLocalizedString(), "Adrian.Page@tesco.net");
    aboutData->addAuthor(ki18n("Adam Celarek"), KLocalizedString(), "tuobaatirk@xibo.at");
    aboutData->addAuthor(ki18n("Alan Horkan"), KLocalizedString(), "", "http://www.openclipart.org");
    aboutData->addAuthor(ki18n("Bart Coppens"), KLocalizedString(), "kde@bartcoppens.be");
    aboutData->addAuthor(ki18n("Boudewijn Rempt"), KLocalizedString(), "boud@valdyas.org", "http://www.valdyas.org/fading/index.cgi");
    aboutData->addAuthor(ki18n("Carsten Pfeiffer"), KLocalizedString(), "carpdjih@cetus.zrz.tu-berlin.de");
    aboutData->addAuthor(ki18n("Casper Boemann"), KLocalizedString(), "cbr@boemann.dk");
    aboutData->addAuthor(ki18n("Clarence Dang"), KLocalizedString(), "dang@kde.org");
    aboutData->addAuthor(ki18n("Cyrille Berger"), KLocalizedString(), "cyb@lepi.org");
    aboutData->addAuthor(ki18n("Dirk Schoenberger"), KLocalizedString(), "dirk.schoenberger@sz-online.de");
    aboutData->addAuthor(ki18n("Danny Allen"), KLocalizedString() , "danny@dannyallen.co.uk");
    aboutData->addAuthor(ki18n("Dmitry Kazakov"), KLocalizedString() , "dimula73@gmail.com");
    aboutData->addAuthor(ki18n("Edward Apap"), KLocalizedString() , "schumifer@hotmail.com");
    aboutData->addAuthor(ki18n("Hanisch Elián"), KLocalizedString() , "lambdae2@gmail.com");
    aboutData->addAuthor(ki18n("Emanuele Tamponi"), KLocalizedString(), "emanuele@valinor.it");
    aboutData->addAuthor(ki18n("Gábor Lehel"), KLocalizedString(), "<illissius@gmail.com>");
    aboutData->addAuthor(ki18n("John Califf"), KLocalizedString(), "jcaliff@compuzone.net");
    aboutData->addAuthor(ki18n("Laurent Montel"), KLocalizedString(), "lmontel@mandrakesoft.com");
    aboutData->addAuthor(ki18n("Lukáš Tvrdý"), KLocalizedString(), "lukast.dev@gmail.com");
    aboutData->addAuthor(ki18n("Marc Pegon"), KLocalizedString(), "pe.marc@free.fr");
    aboutData->addAuthor(ki18n("Matthias Elter"), KLocalizedString(), "me@kde.org");
    aboutData->addAuthor(ki18n("Melchior Franz"), KLocalizedString(), "mfranz@kde.org");
    aboutData->addAuthor(ki18n("Michael Koch"), KLocalizedString(), "koch@kde.org");
    aboutData->addAuthor(ki18n("Michael Thaler"), KLocalizedString(), "michael.thaler@physik.tu-muenchen.de");
    aboutData->addAuthor(ki18n("Patrick Julien"), KLocalizedString(), "freak@codepimps.org");
    aboutData->addAuthor(ki18n("Roger Larsson"), KLocalizedString(), "roger.larsson@norran.net");
    aboutData->addAuthor(ki18n("Samy Lange"), KLocalizedString(), "enkithan@free.fr");
    aboutData->addAuthor(ki18n("Sven Langkamp"), KLocalizedString(), "sven.langkamp@gmail.com");
    aboutData->addAuthor(ki18n("Toshitaka Fujioka"), KLocalizedString(), "fujioka@kde.org");
    aboutData->addAuthor(ki18n("Thomas Zander"), KLocalizedString(), "zander@kde.org");
    aboutData->addAuthor(ki18n("Tom Burdick"), KLocalizedString(), "tburdi1@uic.edu");
    aboutData->addAuthor(ki18n("Vera Lukman"), KLocalizedString(), "shichan.karachu@gmail.com");
    aboutData->addAuthor(ki18n("Sander Koning"), KLocalizedString(), "sanderkoning@kde.nl");
    aboutData->addAuthor(ki18n("Andreas Lundin"), KLocalizedString(), "adde@update.uu.se");
    return aboutData;
}

#endif // KIS_ABOUT_DATA_H_
