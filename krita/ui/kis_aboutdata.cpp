/*
 *  Copyright (c) 1999-2000 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003-2014 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_aboutdata.h"

#include <klocale.h>
#include <calligraversion.h>
#include <calligragitversion.h>
#include <KoIcon.h>

K4AboutData *newKritaAboutData()
{
    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version;

#ifdef CALLIGRA_GIT_SHA1_STRING
    QString gitVersion(CALLIGRA_GIT_SHA1_STRING);
    version = QString("%1 (git %2)").arg(calligraVersion).arg(gitVersion);
#else
    version = calligraVersion;
#endif

    K4AboutData * aboutData = new K4AboutData("krita", 0,
                                            ki18n("Krita"),
                                            version.toLatin1(),
                                            ki18n("Digital Painting for Artists"),
                                            K4AboutData::License_LGPL,
                                            ki18n("© 1999-%1, The Krita Team").subs(CALLIGRA_YEAR),
                                            KLocalizedString(),
                                            "http://www.krita.org",
                                            "submit@bugs.kde.org");

    aboutData->setProgramIconName(koIconName("calligrakrita"));
    return aboutData;
}

