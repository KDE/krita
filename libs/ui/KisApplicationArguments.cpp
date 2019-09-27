/*
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KisApplicationArguments.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QDataStream>
#include <QBuffer>

#include <klocalizedstring.h>
#include <KisPart.h>
#include <KisDocument.h>

struct Q_DECL_HIDDEN KisApplicationArguments::Private
{
    Private()
    {
    }

    QStringList filenames;
    int dpiX {72};
    int dpiY {72};
    bool doTemplate {false};
    bool exportAs {false};
    bool exportSequence {false};
    QString exportFileName;
    QString workspace;
    QString windowLayout;
    QString session;
    bool canvasOnly {false};
    bool noSplash {false};
    bool fullScreen {false};

    bool newImage {false};
    QString colorModel {"RGBA"};
    QString colorDepth {"U8"};
    int width {2000};
    int height {5000};
};


KisApplicationArguments::KisApplicationArguments()
    : d(new Private)
{
}


KisApplicationArguments::~KisApplicationArguments()
{
}

KisApplicationArguments::KisApplicationArguments(const QApplication &app)
    : d(new Private)
{
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("template"), i18n("Open a new document with a template")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("new-image"), i18n("Create a new image on startup.\n"
                                                                                          "Possible colorspace values are:\n"
                                                                                          "    * RGBA\n"
                                                                                          "    * XYZA\n"
                                                                                          "    * LABA\n"
                                                                                          "    * CMYKA\n"
                                                                                          "    * GRAY\n"
                                                                                          "    * YCbCrA\n"
                                                                                          "Possible channel depth arguments are\n"
                                                                                          "    * U8 (8 bits integer)\n"
                                                                                          "    * U16 (16 bits integer)\n"
                                                                                          "    * F16 (16 bits floating point)\n"
                                                                                          "    * F32 (32 bits floating point)\n"),
                                        QLatin1String("colorspace,depth,width,height")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("workspace"), i18n("The name of the workspace to open Krita with"), QLatin1String("workspace")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("windowlayout"), i18n("The name of the window layout to open Krita with"), QLatin1String("windowlayout")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("load-session"), i18n("The name of the session to open Krita with"), QLatin1String("load-session"))); // NB: the argument "session" is already used by QGuiApplication
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("canvasonly"), i18n("Start Krita in canvas-only mode")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("nosplash"), i18n("Do not show the splash screen")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("fullscreen"), i18n("Start Krita in full-screen mode")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("dpi"), i18n("Override display DPI"), QLatin1String("dpiX,dpiY")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export"), i18n("Export to the given filename and exit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export-sequence"), i18n("Export animation to the given filename and exit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export-filename"), i18n("Filename for export"), QLatin1String("filename")));
    parser.addPositionalArgument(QLatin1String("[file(s)]"), i18n("File(s) or URL(s) to open"));
    parser.process(app);

    QString dpiValues = parser.value("dpi");
    if (!dpiValues.isEmpty()) {
        int sep = dpiValues.indexOf(QRegExp("[x, ]"));
        bool ok = true;
        if (sep != -1) {
            d->dpiY = dpiValues.mid(sep + 1).toInt(&ok);
            dpiValues.truncate(sep);
        }
        if (ok) {
            d->dpiX = dpiValues.toInt(&ok);
            if (ok) {
                if (!d->dpiY)
                    d->dpiY = d->dpiX;
            }
        }
    }

    QString newImageValues = parser.value("new-image");
    d->newImage = !newImageValues.isEmpty();
    if (d->newImage) {
        QStringList v = newImageValues.split(",");
        if (v.size() != 4) {
            d->newImage = false;
            qWarning() << "Cannot create a new image: please specify colormodel, depth, width and height.";
        }
        d->colorModel = v[0].toUpper();
        d->colorDepth = v[1].toUpper();
        d->width = v[2].toInt();
        d->height = v[3].toInt();
    }


    d->exportFileName = parser.value("export-filename");
    d->workspace = parser.value("workspace");
    d->windowLayout = parser.value("windowlayout");
    d->session = parser.value("load-session");
    d->doTemplate = parser.isSet("template");
    d->exportAs = parser.isSet("export");
    d->exportSequence = parser.isSet("export-sequence");
    d->canvasOnly = parser.isSet("canvasonly");
    d->noSplash = parser.isSet("nosplash");
    d->fullScreen = parser.isSet("fullscreen");

    const QDir currentDir = QDir::current();
    Q_FOREACH (const QString &filename, parser.positionalArguments()) {
        d->filenames << currentDir.absoluteFilePath(filename);
    }
}

KisApplicationArguments::KisApplicationArguments(const KisApplicationArguments &rhs)
    : d(new Private)
{
    d->filenames = rhs.filenames();
    d->dpiX = rhs.dpiX();
    d->dpiY = rhs.dpiY();
    d->doTemplate = rhs.doTemplate();
    d->exportAs = rhs.exportAs();
    d->exportFileName = rhs.exportFileName();
    d->canvasOnly = rhs.canvasOnly();
    d->workspace = rhs.workspace();
    d->windowLayout = rhs.windowLayout();
    d->session = rhs.session();
    d->noSplash = rhs.noSplash();
    d->fullScreen = rhs.fullScreen();
}

void KisApplicationArguments::operator=(const KisApplicationArguments &rhs)
{
    d->filenames = rhs.filenames();
    d->dpiX = rhs.dpiX();
    d->dpiY = rhs.dpiY();
    d->doTemplate = rhs.doTemplate();
    d->exportAs = rhs.exportAs();
    d->exportFileName = rhs.exportFileName();
    d->canvasOnly = rhs.canvasOnly();
    d->workspace = rhs.workspace();
    d->windowLayout = rhs.windowLayout();
    d->session = rhs.session();
    d->noSplash = rhs.noSplash();
    d->fullScreen = rhs.fullScreen();
}

QByteArray KisApplicationArguments::serialize()
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    QDataStream ds(&buf);
    ds.setVersion(QDataStream::Qt_5_0);
    ds << d->filenames.count();
    Q_FOREACH (const QString &filename, d->filenames) {
        ds << filename;
    }
    ds << d->dpiX;
    ds << d->dpiY;
    ds << d->doTemplate;
    ds << d->exportAs;
    ds << d->exportFileName;
    ds << d->workspace;
    ds << d->windowLayout;
    ds << d->session;
    ds << d->canvasOnly;
    ds << d->noSplash;
    ds << d->fullScreen;
    ds << d->newImage;
    ds << d->height;
    ds << d->width;
    ds << d->height;
    ds << d->colorModel;
    ds << d->colorDepth;



    buf.close();

    return ba;
}

KisApplicationArguments KisApplicationArguments::deserialize(QByteArray &serialized)
{
    KisApplicationArguments args;

    QBuffer buf(&serialized);
    buf.open(QIODevice::ReadOnly);
    QDataStream ds(&buf);
    ds.setVersion(QDataStream::Qt_5_0);
    int count;
    ds >> count;
    for(int i = 0; i < count; ++i) {
        QString s;
        ds >> s;
        args.d->filenames << s;
    }
    ds >> args.d->dpiX;
    ds >> args.d->dpiY;
    ds >> args.d->doTemplate;
    ds >> args.d->exportAs;
    ds >> args.d->exportFileName;
    ds >> args.d->workspace;
    ds >> args.d->windowLayout;
    ds >> args.d->session;
    ds >> args.d->canvasOnly;
    ds >> args.d->noSplash;
    ds >> args.d->fullScreen;
    ds >> args.d->newImage;
    ds >> args.d->height;
    ds >> args.d->width;
    ds >> args.d->height;
    ds >> args.d->colorModel;
    ds >> args.d->colorDepth;

    buf.close();

    return args;
}

QStringList KisApplicationArguments::filenames() const
{
    return d->filenames;
}

int KisApplicationArguments::dpiX() const
{
    return d->dpiX;
}

int KisApplicationArguments::dpiY() const
{
    return d->dpiY;
}

bool KisApplicationArguments::doTemplate() const
{
    return d->doTemplate;
}

bool KisApplicationArguments::exportAs() const
{
    return d->exportAs;
}

bool KisApplicationArguments::exportSequence() const
{
    return d->exportSequence;
}

QString KisApplicationArguments::exportFileName() const
{
    return d->exportFileName;
}

QString KisApplicationArguments::workspace() const
{
    return d->workspace;
}

QString KisApplicationArguments::windowLayout() const
{
    return d->windowLayout;
}

QString KisApplicationArguments::session() const
{
    return d->session;
}

bool KisApplicationArguments::canvasOnly() const
{
    return d->canvasOnly;
}

bool KisApplicationArguments::noSplash() const
{
    return d->noSplash;
}

bool KisApplicationArguments::fullScreen() const
{
    return d->fullScreen;
}

bool KisApplicationArguments::doNewImage() const
{
    return d->newImage;
}

KisDocument *KisApplicationArguments::image() const
{
    KisDocument *doc = KisPart::instance()->createDocument();
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(d->colorModel, d->colorDepth, "");
    if (!cs) {
        qWarning() << "Could not create the colorspace for the new image. Check the colorspace and depth arguments.";
        return 0;
    }

    doc->newImage(i18n("Unnamed"), d->width, d->height, cs, KoColor(QColor(Qt::white), cs), KisConfig::CANVAS_COLOR, 1, "", 100.0);
    return doc;
}
