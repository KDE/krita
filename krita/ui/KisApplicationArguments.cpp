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
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QDataStream>
#include <QBuffer>

#include <klocalizedstring.h>

struct Q_DECL_HIDDEN KisApplicationArguments::Private
{
    Private()
        : dpiX(0)
        , dpiY(0)
        , doTemplate(false)
        , print(false)
        , exportAs(false)
        , exportAsPdf(false)
    {
    }

    QStringList filenames;
    int dpiX;
    int dpiY;
    bool doTemplate;
    bool print;
    bool exportAs;
    bool exportAsPdf;
    QString exportFileName;
    QString profileFileName;

};

KisApplicationArguments::KisApplicationArguments(const QApplication &app)
    : d(new Private)
{
    qDebug() << "Going to parse" << app.arguments();

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("print"), i18n("Only print and exit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("template"), i18n("Open a new document with a template")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("dpi"), i18n("Override display DPI"), QLatin1String("dpiX,dpiY")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export-pdf"), i18n("Only export to PDF and exit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export"), i18n("Export to the given filename and exit")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("export-filename"), i18n("Filename for export/export-pdf"), QLatin1String("filename")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("profile-filename"), i18n("Filename to write profiling information into."), QLatin1String("filename")));
    parser.addPositionalArgument(QLatin1String("[file(s)]"), i18n("File(s) or URL(s) to open"));
    parser.process(app);

    d->filenames = parser.positionalArguments();

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
    d->doTemplate = parser.isSet("template");
    d->print = parser.isSet("print");
    d->exportAs = parser.isSet("export");
    d->exportAsPdf = parser.isSet("export-pdf");
    d->exportFileName = parser.value("export-filename");
    d->profileFileName = parser.value("profile-filename");

    qDebug() << d->filenames;

}

KisApplicationArguments::KisApplicationArguments(const KisApplicationArguments &rhs)
    : d(new Private)
{
    d->filenames = rhs.filenames();
    d->dpiX = rhs.dpiX();
    d->dpiY = rhs.dpiY();
    d->doTemplate = rhs.doTemplate();
    d->print = rhs.print();
    d->exportAs = rhs.exportAs();
    d->exportAsPdf = rhs.exportAsPdf();
    d->exportFileName = rhs.exportFileName();
    d->profileFileName = rhs.profileFileName();
}

KisApplicationArguments::~KisApplicationArguments()
{
}

void KisApplicationArguments::operator=(const KisApplicationArguments &rhs)
{
    d->filenames = rhs.filenames();
    d->dpiX = rhs.dpiX();
    d->dpiY = rhs.dpiY();
    d->doTemplate = rhs.doTemplate();
    d->print = rhs.print();
    d->exportAs = rhs.exportAs();
    d->exportAsPdf = rhs.exportAsPdf();
    d->exportFileName = rhs.exportFileName();
    d->profileFileName = rhs.profileFileName();
}

QByteArray KisApplicationArguments::serialize()
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    QDataStream ds(&buf);
    ds.setVersion(QDataStream::Qt_5_0);
    ds << d->filenames.count();
    foreach(const QString &filename, d->filenames) {
        ds << filename;
    }
    ds << d->dpiX;
    ds << d->dpiY;
    ds << d->doTemplate;
    ds << d->print;
    ds << d->exportAs;
    ds << d->exportAsPdf;
    ds << d->exportFileName;
    ds << d->profileFileName;

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
    ds >> args.d->print;
    ds >> args.d->exportAs;
    ds >> args.d->exportAsPdf;
    ds >> args.d->exportFileName;
    ds >> args.d->profileFileName;

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

bool KisApplicationArguments::print() const
{
    return d->print;
}

bool KisApplicationArguments::exportAs() const
{
    return d->exportAs;
}

bool KisApplicationArguments::exportAsPdf() const
{
    return d->exportAsPdf;
}

QString KisApplicationArguments::exportFileName() const
{
    return d->exportFileName;
}

QString KisApplicationArguments::profileFileName() const
{
    return d->profileFileName;
}

KisApplicationArguments::KisApplicationArguments()
    : d(new Private)
{
}
