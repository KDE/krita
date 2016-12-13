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
Boston, MA 02110-1301, USA.
*/

#include "KisFilterEntry.h"

#include "KisDocument.h"
#include "KisImportExportFilter.h"

#include <kis_debug.h>
#include <KoJsonTrader.h>
#include <kpluginfactory.h>
#include <QFile>
#include <limits.h> // UINT_MAX


KisFilterEntry::KisFilterEntry(QPluginLoader *loader)
    : KisShared()
    , m_loader(loader)
{
    import = loader->metaData().value("MetaData").toObject().value("X-KDE-Import").toString().split(',');
    export_ = loader->metaData().value("MetaData").toObject().value("X-KDE-Export").toString().split(',');
    int w = loader->metaData().value("MetaData").toObject().value("X-KDE-Weight").toString().toInt();
    weight = w < 0 ? UINT_MAX : static_cast<unsigned int>(w);
    available = loader->metaData().value("MetaData").toObject().value("X-KDE-Available").toString();
}

KisFilterEntry::~KisFilterEntry()
{
    delete m_loader;
}

QList<KisFilterEntrySP> KisFilterEntry::query()
{
    QList<KisFilterEntrySP> lst;

    QList<QPluginLoader *> offers = KoJsonTrader::instance()->query("Krita/FileFilter", QString());
    unsigned int max = offers.count();
    dbgFile <<"Query returned" << max <<" offers";

    Q_FOREACH(QPluginLoader *pluginLoader, offers) {
        //dbgFile <<"   desktopEntryPath=" << (*it)->entryPath()
        //               << "   library=" << (*it)->library() << endl;
        // Append converted offer
        lst.append(KisFilterEntrySP(new KisFilterEntry(pluginLoader)));
    }
    return lst;
}

KisImportExportFilter* KisFilterEntry::createFilter()
{
    KLibFactory *factory = qobject_cast<KLibFactory *>(m_loader->instance());

    if (!factory) {
        warnUI << m_loader->errorString();
        return 0;
    }

    QObject *obj = factory->create<KisImportExportFilter>(0);
    if (!obj || !obj->inherits("KisImportExportFilter")) {
        delete obj;
        return 0;
    }

    KisImportExportFilter* filter = static_cast<KisImportExportFilter*>(obj);
    return filter;
}

