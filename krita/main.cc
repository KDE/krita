/*
 *  main.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 */

#include <stdlib.h>

#include <QString>
#include <QPixmap>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QDir>
#include <QDate>
#include <QDebug>

#include <kcmdlineargs.h>

#include <KisApplication.h>
#include <KoConfig.h>

#include <krita_export.h>

#include "data/splash/splash_screen.xpm"
#include "data/splash/splash_holidays.xpm"
#include "ui/kis_aboutdata.h"
#include "ui/kis_factory2.h"
#include "ui/KisDocument.h"
#include "kis_splash_screen.h"
#include "KisPart.h"

#if defined Q_OS_WIN
#include <Windows.h>
#include <stdlib.h>
#include <ui/input/wintab/kis_tablet_support_win.h>
#ifdef USE_BREAKPAD
    #include "kis_crash_handler.h"
#endif
#elif defined Q_WS_X11
#include <ui/input/wintab/kis_tablet_support_x11.h>
#if QT_VERSION < 0x040800
// needed for XInitThreads()
#include <X11/Xlib.h>
#endif

#endif

extern "C" int main(int argc, char **argv)
{
    bool runningInKDE = !qgetenv("KDE_FULL_SESSION").isEmpty();

#ifdef Q_WS_X11
    if (runningInKDE) {
        qputenv("QT_NO_GLIB", "1");
    }
#endif

#ifdef USE_BREAKPAD
    qputenv("KDE_DEBUG", "1");
    KisCrashHandler crashHandler;
    Q_UNUSED(crashHandler);
#endif

#if defined Q_OS_WIN
    SetProcessDPIAware(); // The n-trig wintab driver needs this to report the correct dimensions
#endif

    int state;
    KAboutData *aboutData = KisFactory::aboutData();

    KCmdLineArgs::init(argc, argv, aboutData);

    KCmdLineOptions options;
    options.add("print", ki18n("Only print and exit"));
    options.add("template", ki18n("Open a new document with a template"));
    options.add("dpi <dpiX,dpiY>", ki18n("Override display DPI"));
    options.add("export-pdf", ki18n("Only export to PDF and exit"));
    options.add("export", ki18n("Export to the given filename and exit"));
    options.add("export-filename <filename>", ki18n("Filename for export/export-pdf"));
    options.add("profile-filename <filename>", ki18n("Filename to write profiling information into."));
    options.add("+[file(s)]", ki18n("File(s) or URL(s) to open"));

    KCmdLineArgs::addCmdLineOptions(options);

    // A per-user unique string, without /, because QLocalServer cannot use names with a / in it
    QString key = "Krita" +
                  QDesktopServices::storageLocation(QDesktopServices::HomeLocation).replace("/", "_");
    key = key.replace(":", "_").replace("\\","_");

#if defined Q_WS_X11
#if QT_VERSION >= 0x040800
    // we need to call XInitThreads() (which this does) because of gmic (and possibly others)
    // do their own X11 stuff in their own threads
    // this call must happen before the creation of the application (see AA_X11InitThreads docs)
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads, true);
#else
    XInitThreads();
#endif
#endif

    // first create the application so we can create a pixmap
    KisApplication app(key);

    // If we should clear the config, it has to be done as soon as possible after
    // KisApplication has been created. Otherwise the config file may have been read
    // and stored in a KConfig object we have no control over.
    app.askClearConfig();

    // create factory only after application, the componentData it creates in the
    // constructor will need an existing QCoreApplication at least with Qt5/KF5,
    // to set name of application etc., as also needed to find resources
    KisFactory factory;
    Q_UNUSED(factory); // Not really, it'll self-destruct on exiting main

    if (app.isRunning()) {

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        // only pass arguments to main instance if they are not for batch processing
        // any batch processing would be done in this separate instance
        const bool batchRun =
            args->isSet("print") || args->isSet("export") || args->isSet("export-pdf");

        if (!batchRun) {

            QByteArray ba;
            QDataStream ds(&ba, QIODevice::WriteOnly);
            args->saveAppArgs(ds);
            ds.device()->close();

            if (app.sendMessage(ba)) {
                return 0;
            }
        }
    }

#if defined Q_OS_WIN
    KisTabletSupportWin::init();
    app.setEventFilter(&KisTabletSupportWin::eventFilter);
#elif defined Q_WS_X11
    KisTabletSupportX11::init();
    app.setEventFilter(&KisTabletSupportX11::eventFilter);
#endif


    if (!runningInKDE) {
        // Icons in menus are ugly and distracting
        app.setAttribute(Qt::AA_DontShowIconsInMenus);
    }

    // then create the pixmap from an xpm: we cannot get the
    // location of our datadir before we've started our components,
    // so use an xpm.
    QDate currentDate = QDate::currentDate();
    QWidget *splash = 0;
    if (currentDate > QDate(currentDate.year(), 12, 4) ||
            currentDate < QDate(currentDate.year(), 1, 9)) {
         splash = new KisSplashScreen(aboutData->version(), QPixmap(splash_holidays_xpm));
    }
    else {
        splash = new KisSplashScreen(aboutData->version(), QPixmap(splash_screen_xpm));
    }

    app.setSplashScreen(splash);

    if (!app.start()) {
        return 1;
    }

    // Set up remote arguments.
    QObject::connect(&app, SIGNAL(messageReceived(QByteArray,QObject*)),
                     &app, SLOT(remoteArguments(QByteArray,QObject*)));

    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)),
                     &app, SLOT(fileOpenRequested(QString)));


    state = app.exec();

    return state;
}

