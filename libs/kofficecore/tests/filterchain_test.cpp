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

#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <kcomponentdata.h>
#include <kdebug.h>
//Added by qt3to4:
#include <Q3CString>

int main( int /*argc*/, char ** /*argv*/ )
{
    KComponentData componentData( "filterchain_test" );  // we need an instance when using the trader
    KOffice::Graph g( "application/x-kspread" );
    g.dump();
    g.setSourceMimeType( "application/x-kword" );
    g.dump();

    KoFilterManager *manager = new KoFilterManager( 0 );
    kDebug() << "Trying to build some filter chains..." << endl;
    Q3CString mimeType( "foo/bar" );
    KoFilterChain::Ptr chain = g.chain( manager, mimeType );
    if ( !chain )
        kDebug() << "Chain for 'foo/bar' is not available, OK" << endl;
    else {
        kError() << "Chain for 'foo/bar' is available!" << endl;
        chain->dump();
    }

    mimeType = "application/x-krita";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kDebug() << "Chain for 'application/x-krita' is not available, OK" << endl;
    else {
        kError() << "Chain 'application/x-krita' is available!" << endl;
        chain->dump();
    }

    mimeType = "text/csv";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kError() << "Chain for 'text/csv' is not available!" << endl;
    else {
        kDebug() << "Chain for 'text/csv' is available, OK" << endl;
        chain->dump();
    }

    // Try to find the closest KOffice part
    mimeType = "";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kDebug() << "It was already a KOffice part, OK" << endl;
    else
        kError() << "We really got a chain? ugh :}" << endl;

    g.setSourceMimeType( "text/csv" );
    mimeType = "";
    chain = g.chain( manager, mimeType );
    if ( !chain )
        kError() << "Hmm... why didn't we find a chain?" << endl;
    else {
        kDebug() << "Chain for 'text/csv' -> closest part is available ("
                  << mimeType << "), OK" << endl;
        chain->dump();
    }

    kDebug() << "Checking mimeFilter() for Import:" << endl;
    QStringList list = KoFilterManager::mimeFilter( "application/x-kword",  KoFilterManager::Import );
    QStringList::ConstIterator it = list.begin();
    QStringList::ConstIterator end = list.end();
    for ( ; it != end; ++it )
        kDebug() << "   " << *it << endl;
    kDebug() << "   " << list.count() << " entries." << endl;

    kDebug() << "Checking mimeFilter() for Export:" << endl;
    list = KoFilterManager::mimeFilter( "application/x-kword",  KoFilterManager::Export );
    it = list.begin();
    end = list.end();
    for ( ; it != end; ++it )
        kDebug() << "   " << *it << endl;
    kDebug() << "   " << list.count() << " entries." << endl;

    kDebug() << "Checking KoShell's mimeFilter():" << endl;
    list = KoFilterManager::mimeFilter();
    it = list.begin();
    end = list.end();
    for ( ; it != end; ++it )
        kDebug() << "   " << *it << endl;
    kDebug() << "   " << list.count() << " entries." << endl;

    delete manager;
    return 0;
}
