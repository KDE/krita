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
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <klocalizedstring.h>

#include <KAboutData>
#include <kis_debug.h>

#include <calligraversion.h>
#include <kritagitversion.h>

#include "mainwindow.h"

int main( int argc, char **argv )
{
    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version;

    KLocalizedString::setApplicationDomain( "crashhandler" );

#ifdef KRITA_GIT_SHA1_STRING
    QString gitVersion(KRITA_GIT_SHA1_STRING);
    version = QString("%1 (git %2)").arg(calligraVersion).arg(gitVersion).toLatin1();
#else
    version = calligraVersion;
#endif
    KAboutData aboutData("krita",
                         i18n("Krita Crash Reporter"),
                         version.toLatin1(),
                         i18n("Digital Painting for Artists"),
                         KAboutLicense::GPL,
                         i18n("(c) 2016 The Krita team.\n"),
                         QString(),
                         "http://www.krita.org",
                         "submit@bugs.kde.org");

    aboutData.addAuthor(i18n("Boudewijn Rempt"),
                        i18n("Maintainer"),
                        "boud@valdyas.org", "http://www.valdyas.org/fading/index.cgi");

    QApplication app(argc, argv);
    KAboutData::setApplicationData(aboutData);
    QCommandLineParser parser;
    QCommandLineOption option("+[arg1]");
    QCommandLineOption option2("+[arg2]");
    parser.addOptions({option, option2});

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QStringList arguments = parser.positionalArguments();

    // Something went wrong, whatever: we restart Krita
    if (arguments.size() != 2)  {
        MainWindow mw("", "");
        mw.restart();
        return 1;
    }

    QString dumpPath = arguments[0];
    QString dumpId = arguments[1];

    MainWindow mw(dumpPath, dumpId);

    mw.show();
    return app.exec();
}
