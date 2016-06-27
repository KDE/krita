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
#include <QTemporaryFile>
#include <kis_debug.h>
#include <QStack>
#include "KisImportExportManager.h"
#include "KoUpdater.h"
#include <klocalizedstring.h>

class Q_DECL_HIDDEN KisImportExportFilter::Private
{
public:
    QPointer<KoUpdater> updater;

    Private()
        : updater(0)
    {}

    /**
     * Use this pointer to access all information about input/output
     * during the conversion. @em Don't use it in the constructor -
     * it's invalid while constructing the object!
     */
    KisFilterChainSP chain;

};

KisImportExportFilter::KisImportExportFilter(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisDocument *KisImportExportFilter::inputDocument() const
{
    return d->chain->inputDocument();
}

KisDocument *KisImportExportFilter::outputDocument() const
{
    return d->chain->outputDocument();
}

QString KisImportExportFilter::inputFile() const
{
    return d->chain->inputFile();
}

QString KisImportExportFilter::outputFile() const
{
    return d->chain->outputFile();
}

bool KisImportExportFilter::getBatchMode() const
{
    return d->chain->manager()->getBatchMode();
}

KisImportExportFilter::~KisImportExportFilter()
{
    Q_ASSERT(d->updater);
    if (d->updater) d->updater->setProgress(100);
    delete d;
}

void KisImportExportFilter::setChain(KisFilterChainSP chain)
{
    d->chain = chain;
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
    case StupidError: // ?? what is this ??
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
    return 0;
}

KisPropertiesConfigurationSP KisImportExportFilter::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    return defaultConfiguration(from, to);
}

KisConfigWidget *KisImportExportFilter::createConfigurationWidget(QWidget *, const QByteArray &from, const QByteArray &to) const
{
    return 0;
}

void KisImportExportFilter::setUpdater(const QPointer<KoUpdater>& updater)
{
    Q_ASSERT(updater);
    if (d->updater && !updater) {
        disconnect(this, SLOT(slotProgress(int)));
    } else if (!d->updater && updater) {
        connect(this, SIGNAL(sigProgress(int)), SLOT(slotProgress(int)));
    }
    d->updater = updater;
}

void KisImportExportFilter::slotProgress(int value)
{
    Q_ASSERT(d->updater);
    if (d->updater) {
        d->updater->setValue(value);
    }
}
