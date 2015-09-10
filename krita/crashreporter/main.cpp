/* This file is part of the KDE project
   Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QTextStream>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kis_debug.h>

#include <calligraversion.h>
#include <calligragitversion.h>

#include "mainwindow.h"

int main( int argc, char **argv )
{
    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version;

    KLocalizedString::setApplicationDomain( "crashhandler" );

#ifdef CALLIGRA_GIT_SHA1_STRING
    QString gitVersion(CALLIGRA_GIT_SHA1_STRING);
    version = QString("%1 (git %2)").arg(calligraVersion).arg(gitVersion).toLatin1();
#else
    version = calligraVersion;
#endif
    KAboutData aboutData("krita", 0,
                         ki18n("Krita Crash Reporter"),
                         version.toLatin1(),
                         ki18n("Digital Painting for Artists"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2014 The Krita team.\n"),
                         KLocalizedString(),
                         "http://www.krita.org",
                         "submit@bugs.kde.org");

    aboutData.addAuthor(ki18n("Boudewijn Rempt"), 
                        ki18n("Maintainer"), 
                        "boud@valdyas.org", "http://www.valdyas.org/fading/index.cgi");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[arg1]");
    options.add("+[arg2]");
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app;

    QStringList arguments = args->allArguments();

    // Something went wrong, whatever: we restart Krita
    if (arguments.size() != 3)  {
        MainWindow mw("", "");
        mw.restart();
    }

    QString dumpPath = arguments[1];
    QString dumpId = arguments[2];

    MainWindow mw(dumpPath, dumpId);

    mw.show();
    return app.exec();
}
