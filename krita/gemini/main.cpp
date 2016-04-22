/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QStringList>
#include <QString>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDir>
#include <QSplashScreen>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <KoResourcePaths.h>
#include <kiconloader.h>
#include "data/splash/splash_screen.xpm"
#include "MainWindow.h"

#include "sketch/SketchInputContext.h"

#include <kritaversion.h>
#include <kritagitversion.h>

#if defined Q_OS_WIN
#include "stdlib.h"
#include <ui/input/wintab/kis_tablet_support_win.h>
#elif defined HAVE_X11
#include <ui/input/wintab/kis_tablet_support_x11.h>
#endif


int main( int argc, char** argv )
{
    QString kritaVersion(KRITA_VERSION_STRING);
    QString version;


#ifdef KRITA_GIT_SHA1_STRING
    QString gitVersion(KRITA_GIT_SHA1_STRING);
    version = QString("%1 (git %2)").arg(kritaVersion).arg(gitVersion).toLatin1();
#else
    version = kritaVersion;
#endif


    KAboutData aboutData("kritagemini",
                         "krita",
                         ki18n("Krita Gemini"),
                         version.toLatin1(),
                         ki18n("Krita Gemini: Painting at Home and on the Go for Artists"),
                         KAboutData::License_GPL,
                         ki18n("(c) 1999-%1 The Krita team.\n").subs(KRITA_YEAR),
                         KLocalizedString(),
                         "http://www.kritastudio.com",
                         "submit@bugs.kde.org");

    KCmdLineArgs::init (argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add( "+[files]", ki18n( "Images to open" ) );
    options.add( "vkb", ki18n( "Use the virtual keyboard" ) );
    options.add( "fullscreen", ki18n( "Use full-screen display" ) );
    options.add( "sketch", ki18n( "Start with the Sketch interface" ) );
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    QStringList fileNames;
    if (args->count() > 0) {
        for (int i = 0; i < args->count(); ++i) {
            QString fileName = args->arg(i);
            if (QFile::exists(fileName)) {
                fileNames << fileName;
            }
        }
    }

    KApplication app;
    app.setApplicationName("kritagemini");
    KIconLoader::global()->addAppDir("krita");
    KIconLoader::global()->addAppDir("kritasketch");

#ifdef Q_OS_WIN
    QDir appdir(app.applicationDirPath());
    appdir.cdUp();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // If there's no kdehome, set it and restart the process.
    if (!env.contains("KDEHOME") ) {
        _putenv_s("KDEHOME", QDesktopServices::storageLocation(QDesktopServices::DataLocation).toLocal8Bit());
    }
    if (!env.contains("KDESYCOCA")) {
        _putenv_s("KDESYCOCA", QString(appdir.absolutePath() + "/sycoca").toLocal8Bit());
    }
    if (!env.contains("XDG_DATA_DIRS")) {
        _putenv_s("XDG_DATA_DIRS", QString(appdir.absolutePath() + "/share").toLocal8Bit());
    }
    if (!env.contains("KDEDIR")) {
        _putenv_s("KDEDIR", appdir.absolutePath().toLocal8Bit());
    }
    if (!env.contains("KDEDIRS")) {
        _putenv_s("KDEDIRS", appdir.absolutePath().toLocal8Bit());
    }
    _putenv_s("PATH", QString(appdir.absolutePath() + "/bin" + ";"
              + appdir.absolutePath() + "/lib" + ";"
              + appdir.absolutePath() + "/lib"  +  "/kde4" + ";"
              + appdir.absolutePath()).toLocal8Bit());

    app.addLibraryPath(appdir.absolutePath());
    app.addLibraryPath(appdir.absolutePath() + "/bin");
    app.addLibraryPath(appdir.absolutePath() + "/lib");
    app.addLibraryPath(appdir.absolutePath() + "/lib/kde4");
#endif

#if defined Q_OS_WIN
    KisTabletSupportWin::init();
    app.setEventFilter(&KisTabletSupportWin::eventFilter);
#elif defined HAVE_X11
    KisTabletSupportX11::init();
    app.setEventFilter(&KisTabletSupportX11::eventFilter);
#endif
	
	if (qgetenv("KDE_FULL_SESSION").isEmpty()) {
        // There are two themes that work for Krita, oxygen and plastique. Try to set plastique first, then oxygen
        qobject_cast<QApplication*>(QApplication::instance())->setStyle("Plastique");
		qobject_cast<QApplication*>(QApplication::instance())->setStyle("Oxygen");
    }

    // Prepare to show window fullscreen if required
    bool showFullscreen = false;

#ifdef HAVE_STEAMWORKS
    if (steamClient->isInBigPictureMode()) {
        // Show main window full screen
        showFullscreen = true;
    }
#endif

    if (args->isSet("sketch")) {
        showFullscreen = true;
    }

	if (args->isSet("fullscreen")) {
        showFullscreen = true;
    }

    // then create the pixmap from an xpm: we cannot get the
    // location of our datadir before we've started our components,
    // so use an xpm.
    // If fullscreen, hide splash screen
    QPixmap pm(splash_screen_xpm);
    QSplashScreen splash(pm);
    if (!showFullscreen) {
        splash.show();
        splash.showMessage(".");
        app.processEvents();
    }

#if defined HAVE_X11
    QApplication::setAttribute(Qt::AA_X11InitThreads);
#endif

    MainWindow window(fileNames);

    if (args->isSet("vkb")) {
        app.setInputContext(new SketchInputContext(&app));
    }

    if (args->isSet("sketch")) {
        window.setSlateMode(true);
    }

    if (showFullscreen) {
        window.showFullScreen();
    } else {
#ifdef Q_OS_WIN
		window.showMaximized();
#else
		window.show();
#endif
	}
    splash.finish(&window);

    return app.exec();
}
