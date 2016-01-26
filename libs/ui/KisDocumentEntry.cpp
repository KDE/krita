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

#include <kis_debug.h>
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
        warnUI << "Got no results with " << mimetype;
        // Fallback to the old way (which was probably wrong, but better be safe)
        vec = query(mimetype);

        if (vec.isEmpty()) {
            errUI << "Could not find a plugin to load" << mimetype;
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
    const QList<QPluginLoader *> offers = KoJsonTrader::instance()->query("Calligra/Part", mimetype);

    Q_FOREACH (QPluginLoader *pluginLoader, offers) {
        lst.append(KisDocumentEntry(pluginLoader));
    }

    if (lst.count() > 1 && !mimetype.isEmpty()) {
        warnUI << "KisDocumentEntry::query " << mimetype << " got " << lst.count() << " offers!";
        Q_FOREACH (const KisDocumentEntry &entry, lst) {
            dbgKrita << entry.name();
        }
    }

    return lst;
}
