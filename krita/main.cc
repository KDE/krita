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

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <ksplashscreen.h>
#include <ksycoca.h>

#include <KoApplication.h>

#include <krita_export.h>

#include "data/splash/splash_screen.xpm"
#include "ui/kis_aboutdata.h"
#include "image/brushengine/kis_paintop_registry.h>

#include <Vc/global.h>
#include <Vc/support.h>

#ifdef Q_OS_WIN
#include "stdlib.h"
#endif

static void fatalError(const QString &message) {
    qCritical() << "Fatal Error:" << message;

    if (QMessageBox::critical(0, "Configuration Issue",
                QString("Configuration for Krita has issues.\n"
                "(Details: %1)\n"
                "Shall we continue anyways?").arg(message),
                QMessageBox::Yes|QMessageBox::No)
            == QMessageBox::No) {
        qFatal("aborting due to configuration issues");
    }
}

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
#ifdef Q_WS_X11
    setenv("QT_NO_GLIB", "1", true);
#endif

    int state;
    KAboutData *aboutData = newKritaAboutData();

    KCmdLineArgs::init(argc, argv, aboutData);

    KCmdLineOptions options;
    options.add("+[file(s)]", ki18n("File(s) or URL(s) to open"));
    options.add( "hwinfo", ki18n( "Show some information about the hardware" ));
    KCmdLineArgs::addCmdLineOptions(options);

    // first create the application so we can create a  pixmap
    KoApplication app;

    if (args->isSet("hwinfo")) {
        QString hwinfo;
        QTextStream stst(&hwinfo);
        if (Vc::isImplementationSupported(Vc::SSE2Impl)) {
            stst << "Vc::SSE2Impl: " << Vc::CpuId::hasSse2() << "\n";
        }
        if (Vc::isImplementationSupported(Vc::SSE3Impl)) {
            stst << "Vc::SSE3Impl: " << Vc::CpuId::hasSse3() << "\n";
        }
        if (Vc::isImplementationSupported(Vc::SSSE3Impl)) {
            stst << "Vc::SSSE3Impl: " << Vc::CpuId::hasSsse3() << "\n";
        }
        if (Vc::isImplementationSupported(Vc::SSE41Impl)) {
            stst << "Vc::SSE41Impl: " << Vc::CpuId::hasSse41() << "\n";
        }
        if (Vc::isImplementationSupported(Vc::SSE42Impl)) {
            stst << "Vc::SSE42Impl: " << Vc::CpuId::hasSse42() << "\n";
        }
        if (Vc::isImplementationSupported(Vc::AVXImpl)) {
            stst << "Vc::AVXImpl: " << (Vc::CpuId::hasOsxsave()
                                    && Vc::CpuId::hasAvx()) << "\n";
        }
        if (Vc::isImplementationSupported(Vc::AVX2Impl)) {
            stst << "Vc::AVX2Impl: " << false << "\n";
        }
        QMessageBox::information(0, "hwinfo", hwinfo);
        qApp->quit();
        // quit() is not good enough to terminate here.
        qFatal("hwinfo");
    }


#ifdef Q_WS_X11
    app.setAttribute(Qt::AA_X11InitThreads, true);
#endif

    // assert krita.rc
    QString krita_rc_check = KStandardDirs::locate("data", "krita/krita.rc");
    qDebug() << "KStandardDirs::locate(data, krita.rc):" << krita_rc_check;
    if (krita_rc_check.isNull() || krita_rc_check.isEmpty()) {
        fatalError("rc missing");
    }

    KisPaintOpRegistry *reg = KisPaintOpRegistry::instance();
    if (!reg) bark("KisPaintOpRegistry missing");

    // we should have some paintops by now; if not - terminate gracefully
    // (instead of crashing later inside kritasketch)
    if (reg->listKeys().empty()) {
        fatalError("paintops missing");
    }

    // then create the pixmap from an xpm: we cannot get the
    // location of our datadir before we've started our components,
    // so use an xpm.
    QSplashScreen *splash = new KSplashScreen(QPixmap(splash_screen_xpm));
    app.setSplashScreen(splash);


    if (!app.start()) {
        return 1;
    }

    // now save some memory.
    app.setSplashScreen(0);
    delete splash;

    state = app.exec();

    delete aboutData;

    return state;
}

