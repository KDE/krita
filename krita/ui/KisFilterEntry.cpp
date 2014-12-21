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

#include <kservicetype.h>
#include <kdebug.h>
#include <KoServiceLocator.h>
#include <QFile>

#include <limits.h> // UINT_MAX


KisFilterEntry::KisFilterEntry(const KService::Ptr& service)
        : m_service(service)
{
    import = service->property("X-KDE-Import", QVariant::StringList).toStringList();
    export_ = service->property("X-KDE-Export", QVariant::StringList).toStringList();
    int w = service->property("X-KDE-Weight", QVariant::Int).toInt();
    weight = w < 0 ? UINT_MAX : static_cast<unsigned int>(w);
    available = service->property("X-KDE-Available", QVariant::String).toString();
}

QList<KisFilterEntry::Ptr> KisFilterEntry::query()
{
    QList<KisFilterEntry::Ptr> lst;

    const KService::List offers = KoServiceLocator::instance()->entries("Krita/FileFilter");

    KService::List::ConstIterator it = offers.constBegin();
    unsigned int max = offers.count();
    kDebug(30500) <<"Query returned" << max <<" offers";
    for (unsigned int i = 0; i < max; i++) {
        kDebug(30500) <<"   desktopEntryPath=" << (*it)->desktopEntryPath()
                       << "   library=" << (*it)->library() << endl;
        // Append converted offer
        lst.append(KisFilterEntry::Ptr(new KisFilterEntry(*it)));
        // Next service
        it++;
    }

    return lst;
}

KisImportExportFilter* KisFilterEntry::createFilter(KisFilterChain* chain, QObject* parent)
{
    KPluginLoader loader(*m_service);
    KLibFactory* factory = loader.factory();

    if (!factory) {
        kWarning(30003) << loader.errorString();
        return 0;
    }

    QObject* obj = factory->create<KisImportExportFilter>(parent);
    if (!obj || !obj->inherits("KisImportExportFilter")) {
        delete obj;
        return 0;
    }

    KisImportExportFilter* filter = static_cast<KisImportExportFilter*>(obj);
    filter->m_chain = chain;
    return filter;
}

