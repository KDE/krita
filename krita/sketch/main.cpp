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

#include <QFile>
#include <QStringList>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDir>
#include <QMessageBox>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include "KoGlobal.h"
#include <KoResourcePaths.h>

#include <kritaversion.h>
#include <kritagitversion.h>
#include <opengl/kis_opengl.h>
#include <kis_icon.h>

#include "MainWindow.h"

// QT5TODO
// #include "SketchInputContext.h"

#include "SketchApplication.h"

#if defined Q_OS_WIN
#include "stdlib.h"
#include <kis_tablet_support_win.h>
#elif defined HAVE_X11
#include <kis_tablet_support_x11.h>
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

    KLocalizedString::setApplicationDomain("krita");

    KAboutData aboutData(QStringLiteral("kritasketch"),
                         i18n("Krita Sketch"),
                         QStringLiteral("0.1"),
                         i18n("Krita Sketch: Painting on the Go for Artists"),
                         KAboutLicense::GPL,
                         i18n("(c) 1999-%1 The Krita team.\n").arg(KRITA_YEAR),
                         QString(),
                         QStringLiteral("https://www.krita.org"),
                         QStringLiteral("submit@bugs.kde.org"));

#if defined HAVE_X11
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
#endif
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    SketchApplication app(argc, argv);
    KAboutData::setApplicationData( aboutData );
    app.setWindowIcon(KisIconUtils::loadIcon("kritasketch"));

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("vkb"), i18n("Use the virtual keyboard")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("windowed"), i18n("Open sketch in a window, otherwise defaults to full-screen")));

    parser.addPositionalArgument(QStringLiteral("[file(s)]"), i18n("Images to open"));

    parser.process(app);

    aboutData.processCommandLine(&parser);

    QStringList fileNames;
    Q_FOREACH (const QString &fileName, parser.positionalArguments()) {
        const QString absoluteFilePath = QDir::current().absoluteFilePath(fileName);
        if (QFile::exists(absoluteFilePath)) {
            fileNames << absoluteFilePath;
        }
    }

    // QT5TODO: untested replacement of KIconLoader::global()->addAppDir("krita");
    QStringList themeSearchPaths = QIcon::themeSearchPaths();
    themeSearchPaths.append(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "pics", QStandardPaths::LocateDirectory));
    QIcon::setThemeSearchPaths(themeSearchPaths);

    // Initialize all Calligra directories etc.
    KoGlobal::initialize();

    // for cursors
    KoResourcePaths::addResourceType("kis_pics", "data", "pics/");

    // for images in the paintop box
    KoResourcePaths::addResourceType("kis_images", "data", "images/");

    KoResourcePaths::addResourceType("icc_profiles", "data", "profiles/");

    KisOpenGL::initialize();

    QDir appdir(app.applicationDirPath());
    appdir.cdUp();

#ifdef Q_OS_WIN
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // If there's no kdehome, set it and restart the process.
    //QMessageBox::information(0, i18nc("@title:window", "Krita sketch", "KDEHOME: " + env.value("KDEHOME"));
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
#elif defined HAVE_X11
    KisTabletSupportX11::init();
    // TODO: who owns the filter object?
    app.installNativeEventFilter(new KisTabletSupportX11());
#endif

    app.start();

    MainWindow window(fileNames);

// QT5TODO
//     if (parser.isSet("vkb")) {
//         app.setInputContext(new SketchInputContext(&app));
//     }

    if (parser.isSet("windowed")) {
        window.show();
    } else {
        window.showFullScreen();
    }

    return app.exec();
}
