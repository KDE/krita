/* This file is part of the KDE project
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

#include "KoFilterChain.h"
#include "KoFilterManager.h"
#include "kcomponentdata.h"
#include <kdebug.h>

int main(int /*argc*/, char ** /*argv*/)
{
    KComponentData componentData("filterchain_test");    // we need an instance when using the trader
    KOfficeFilter::Graph g("application/x-kspread");
    g.dump();
    g.setSourceMimeType("application/x-kword");
    g.dump();

    KoFilterManager *manager = new KoFilterManager(0);
    kDebug() << "Trying to build some filter chains...";
    QByteArray mimeType("foo/bar");
    KoFilterChain::Ptr chain = g.chain(manager, mimeType);
    if (!chain)
        kDebug() << "Chain for 'foo/bar' is not available, OK";
    else {
        kError() << "Chain for 'foo/bar' is available!" << endl;
        chain->dump();
    }

    mimeType = "application/x-krita";
    chain = g.chain(manager, mimeType);
    if (!chain)
        kDebug() << "Chain for 'application/x-krita' is not available, OK";
    else {
        kError() << "Chain 'application/x-krita' is available!" << endl;
        chain->dump();
    }

    mimeType = "text/csv";
    chain = g.chain(manager, mimeType);
    if (!chain)
        kError() << "Chain for 'text/csv' is not available!" << endl;
    else {
        kDebug() << "Chain for 'text/csv' is available, OK";
        chain->dump();
    }

    // Try to find the closest KOffice part
    mimeType = "";
    chain = g.chain(manager, mimeType);
    if (!chain)
        kDebug() << "It was already a KOffice part, OK";
    else
        kError() << "We really got a chain? ugh :}" << endl;

    g.setSourceMimeType("text/csv");
    mimeType = "";
    chain = g.chain(manager, mimeType);
    if (!chain)
        kError() << "Hmm... why didn't we find a chain?" << endl;
    else {
        kDebug() << "Chain for 'text/csv' -> closest part is available ("
        << mimeType << "), OK" << endl;
        chain->dump();
    }

    kDebug() << "Checking mimeFilter() for Import:";
    QStringList list = KoFilterManager::mimeFilter("application/x-kword",  KoFilterManager::Import);
    Q_FOREACH(const QString& it, list)
        kDebug() << "" << it;
    kDebug() << "" << list.count() << " entries.";

    kDebug() << "Checking mimeFilter() for Export:";
    list = KoFilterManager::mimeFilter("application/x-kword",  KoFilterManager::Export);
    Q_FOREACH(const QString& it, list)
        kDebug() << "" << it;
    kDebug() << "" << list.count() << " entries.";

    kDebug() << "Checking KoShell's mimeFilter():";
    list = KoFilterManager::mimeFilter();
    Q_FOREACH(const QString& it, list)
        kDebug() << "" << it;
    kDebug() << "" << list.count() << " entries.";

    delete manager;
    return 0;
}
