/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include <SvgTextEditor.h>

#if defined HAVE_X11
#   include <kis_xi2_event_filter.h>
#endif


extern "C" int main(int argc, char **argv)
{
#if defined HAVE_X11
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication app(argc, argv);
#if defined HAVE_X11
    app.installNativeEventFilter(KisXi2EventFilter::instance());
#endif

    KLocalizedString::setApplicationDomain("svgtexttool");
    KAboutData aboutData(QStringLiteral("svgtexttool"),
                         i18n("Svg Text Editor"),
                         QStringLiteral("1.0"),
                         i18n("Test for the Svg Text Tool"),
                         KAboutLicense::LGPL,
                         i18n("(c) 2017"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    SvgTextEditor *mainWindow = new SvgTextEditor();
    mainWindow->show();
    return app.exec();

}

