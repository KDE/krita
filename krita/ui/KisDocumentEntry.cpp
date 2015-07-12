/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

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

#include "KisDocumentEntry.h"

#include "KisPart.h"
#include "KisDocument.h"

#include <kservicetype.h>
#include <kdebug.h>
#include <KoJsonTrader.h>

#include <QPluginLoader>

#include <limits.h> // UINT_MAX

KisDocumentEntry::KisDocumentEntry()
        : m_loader(0)
{
}

KisDocumentEntry::KisDocumentEntry(QPluginLoader *loader)
        : m_loader(loader)
{
}

KisDocumentEntry::~KisDocumentEntry()
{
}

QString KisDocumentEntry::nativeMimeType()
{
    return QString::fromLatin1(KIS_MIME_TYPE);
}

QStringList KisDocumentEntry::extraNativeMimeTypes()
{
    return QStringList() << KIS_MIME_TYPE;
}


QPluginLoader * KisDocumentEntry::loader() const {
    return m_loader;
}

/**
 * @return TRUE if the service pointer is null
 */
bool KisDocumentEntry::isEmpty() const {
    return (m_loader == 0);
}

/**
 * @return name of the associated service
 */
QString KisDocumentEntry::name() const {
    QJsonObject json = m_loader->metaData().value("MetaData").toObject();
    json = json.value("KPlugin").toObject();
    return json.value("Name").toString();
}

/**
 *  Mimetypes (and other service types) which this document can handle.
 */
QStringList KisDocumentEntry::mimeTypes() const {
    QJsonObject json = m_loader->metaData().value("MetaData").toObject();
    return json.value("MimeType").toString().split(';', QString::SkipEmptyParts);
}

/**
 *  @return TRUE if the document can handle the requested mimetype.
 */
bool KisDocumentEntry::supportsMimeType(const QString & _mimetype) const {
    return mimeTypes().contains(_mimetype);
}

KisDocumentEntry KisDocumentEntry::queryByMimeType(const QString & mimetype)
{
    QList<KisDocumentEntry> vec = query(mimetype);

    if (vec.isEmpty()) {
        kWarning(30003) << "Got no results with " << mimetype;
        // Fallback to the old way (which was probably wrong, but better be safe)
        vec = query(mimetype);

        if (vec.isEmpty()) {
            // Still no match. Either the mimetype itself is unknown, or we have no service for it.
            // Help the user debugging stuff by providing some more diagnostics
            if (!KServiceType::serviceType(mimetype)) {
                kError(30003) << "Unknown Calligra MimeType " << mimetype << "." << endl;
                kError(30003) << "Check your installation (for instance, run 'kde4-config --path mime' and check the result)." << endl;
            } else {
                kError(30003) << "Found no Calligra part able to handle " << mimetype << "!" << endl;
                kError(30003) << "Check your installation (does the desktop file have X-KDE-NativeMimeType and Calligra/Part, did you install Calligra in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
            }
            return KisDocumentEntry();
        }
    }

    // Filthy hack alert -- this'll be properly fixed in the mvc branch.
    if (qApp->applicationName() == "flow" && vec.size() == 2) {
        return KisDocumentEntry(vec[1]);
    }

    return KisDocumentEntry(vec[0]);
}

QList<KisDocumentEntry> KisDocumentEntry::query(const QString & mimetype)
{

    QList<KisDocumentEntry> lst;

    // Query the trader
    const QList<QPluginLoader *> offers = KoJsonTrader::self()->query("Calligra/Part", mimetype);

    foreach(QPluginLoader *pluginLoader, offers) {
        lst.append(KisDocumentEntry(pluginLoader));
    }

    if (lst.count() > 1 && !mimetype.isEmpty()) {
        kWarning(30003) << "KisDocumentEntry::query " << mimetype << " got " << lst.count() << " offers!";
        foreach(const KisDocumentEntry &entry, lst) {
            qDebug() << entry.name();
        }
    }

    return lst;
}
