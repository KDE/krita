/* This file is part of the KDE libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

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
Boston, MA 02110-1301, USA.
*/

#include "KisImportExportFilter.h"

#include <QFile>
#include <kis_debug.h>
#include <QStack>
#include "KisImportExportManager.h"
#include <KisExportCheckBase.h>
#include "KoUpdater.h"
#include <klocalizedstring.h>

class Q_DECL_HIDDEN KisImportExportFilter::Private
{
public:
    QPointer<KoUpdater> updater;
    QByteArray mime;
    QString filename;
    bool batchmode;

    QMap<QString, KisExportCheckBase*> capabilities;

    Private()
        : updater(0)
        , batchmode(false)
    {}

    ~Private()
    {
        qDeleteAll(capabilities);
    }

};

KisImportExportFilter::KisImportExportFilter(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisImportExportFilter::~KisImportExportFilter()
{
    Q_ASSERT(d->updater);
    if (d->updater) {
        d->updater->setProgress(100);
    }
    delete d;
}


QString KisImportExportFilter::filename() const
{
    return d->filename;
}

bool KisImportExportFilter::batchMode() const
{
    return d->batchmode;
}


void KisImportExportFilter::setBatchMode(bool batchmode)
{
    d->batchmode = batchmode;
}

void KisImportExportFilter::setFilename(const QString &filename)
{
    d->filename = filename;
}

void KisImportExportFilter::setMimeType(const QString &mime)
{
    d->mime = mime.toLatin1();
}

QByteArray KisImportExportFilter::mimeType() const
{
    return d->mime;
}

QString KisImportExportFilter::conversionStatusString(ConversionStatus status)
{
    QString msg;
    switch (status) {
    case OK: break;

    case FilterCreationError:
        msg = i18n("Could not create the filter plugin"); break;

    case CreationError:
        msg = i18n("Could not create the output document"); break;

    case FileNotFound:
        msg = i18n("File not found"); break;

    case StorageCreationError:
        msg = i18n("Cannot create storage"); break;

    case BadMimeType:
        msg = i18n("Bad MIME type"); break;

    case EmbeddedDocError:
        msg = i18n("Error in embedded document"); break;

    case WrongFormat:
        msg = i18n("Format not recognized"); break;

    case NotImplemented:
        msg = i18n("Not implemented"); break;

    case ParsingError:
        msg = i18n("Parsing error"); break;

    case PasswordProtected:
        msg = i18n("Document is password protected"); break;

    case InvalidFormat:
        msg = i18n("Invalid file format"); break;

    case InternalError:
    case UnexpectedEOF:
    case UnexpectedOpcode:
    case UsageError:
        msg = i18n("Internal error"); break;

    case OutOfMemory:
        msg = i18n("Out of memory"); break;

    case FilterEntryNull:
        msg = i18n("Empty Filter Plugin"); break;

    case NoDocumentCreated:
        msg = i18n("Trying to load into the wrong kind of document"); break;

    case DownloadFailed:
        msg = i18n("Failed to download remote file"); break;

    case ProgressCancelled:
        msg = i18n("Cancelled by user"); break;

    case BadConversionGraph:

        msg = i18n("Unknown file type"); break;

    case UserCancelled:

        // intentionally we do not prompt the error message here
        break;

    default: msg = i18n("Unknown error"); break;
    }
    return msg;
}

KisPropertiesConfigurationSP KisImportExportFilter::defaultConfiguration(const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    return 0;
}

KisPropertiesConfigurationSP KisImportExportFilter::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    return defaultConfiguration(from, to);
}

KisConfigWidget *KisImportExportFilter::createConfigurationWidget(QWidget *, const QByteArray &from, const QByteArray &to) const
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    return 0;
}

QMap<QString, KisExportCheckBase *> KisImportExportFilter::exportChecks()
{
    qDeleteAll(d->capabilities);
    initializeCapabilities();
    return d->capabilities;
}

void KisImportExportFilter::setUpdater(QPointer<KoUpdater> updater)
{
    d->updater = updater;
}

void KisImportExportFilter::setProgress(int value)
{
    if (d->updater) {
        d->updater->setValue(value);
    }
}

void KisImportExportFilter::initializeCapabilities()
{
    // XXX: Initialize everything to fully supported?
}

void KisImportExportFilter::addCapability(KisExportCheckBase *capability)
{
    d->capabilities[capability->id()] = capability;
}
