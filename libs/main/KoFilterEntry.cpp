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

#include "KoFilterEntry.h"

#include "KoDocument.h"
#include "KoFilter.h"

#include <kdebug.h>
#include <KoServiceLocator.h>
#include <KoJsonTrader.h>
#include <kpluginfactory.h>
#include <QFile>

#include <limits.h> // UINT_MAX


KoFilterEntry::KoFilterEntry(QPluginLoader *loader)
        : m_loader(loader)
{
    import = loader->metaData().value("MetaData").toObject().value("X-KDE-Import").toString().split(',');
    export_ = loader->metaData().value("MetaData").toObject().value("X-KDE-Export").toString().split(',');
    int w = loader->metaData().value("MetaData").toObject().value("X-KDE-Weight").toString().toInt();
    weight = w < 0 ? UINT_MAX : static_cast<unsigned int>(w);
    available = loader->metaData().value("MetaData").toObject().value("X-KDE-Available").toString();
}

QList<KoFilterEntry::Ptr> KoFilterEntry::query()
{
    QList<KoFilterEntry::Ptr> lst;

    QList<QPluginLoader *> offers = KoJsonTrader::self()->query("Calligra/Filter", QString());

    QList<QPluginLoader *>::ConstIterator it = offers.constBegin();
    unsigned int max = offers.count();
    //kDebug(30500) <<"Query returned" << max <<" offers";
    for (unsigned int i = 0; i < max; i++) {
        //kDebug(30500) <<"   desktopEntryPath=" << (*it)->desktopEntryPath()
        //               << "   library=" << (*it)->library() << endl;
        // Append converted offer
        lst.append(KoFilterEntry::Ptr(new KoFilterEntry(*it)));
        // Next service
        it++;
    }

    return lst;
}

KoFilter* KoFilterEntry::createFilter(KoFilterChain* chain, QObject* parent)
{
    KLibFactory *factory = qobject_cast<KLibFactory *>(m_loader->instance());

    if (!factory) {
        kWarning(30003) << m_loader->errorString();
        return 0;
    }

    QObject* obj = factory->create<KoFilter>(parent);
    if (!obj || !obj->inherits("KoFilter")) {
        delete obj;
        return 0;
    }

    KoFilter* filter = static_cast<KoFilter*>(obj);
    filter->m_chain = chain;
    return filter;
}

