/* This file is part of the KDE project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>

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

#include <QFile>
#include <QDir>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <KoStore.h>
#include <kdebug.h>
#include <stdlib.h>

#include <string.h>

namespace {
    const char* const test1 = "This test checks whether we're able to write to some arbitrary directory.\n";
    const char* const testDir = "0";
    const char* const testDirResult = "0/";
    const char* const test2 = "This time we try to append the given relative path to the current dir.\n";
    const char* const test3 = "<xml>Hello World</xml>";
    const char* const testDir2 = "test2/with/a";
    const char* const testDir2Result = "0/test2/with/a/";
    const char* const test4 = "<xml>Heureka, it works</xml>";

    const char* const badStorage = "Ooops, bad storage???";
    const char* const unableToOpen = "Couldn't open storage!";
    const char* const brokenPath = "Path handling broken!";
    const char* const unableToRead = "Couldn't read stream back!";
}

char getch( QIODevice * dev ) {
    char c = 0;
    dev->getChar( &c );
    return c;
}

int cleanUp( KoStore* store, const QString& testFile, const char* error )
{
    QFile::remove( testFile );
    delete store;
    kDebug() << error << endl;
    return 1;
}

#define DATALEN 64

int test( const char* testName, KoStore::Backend backend, const QString& testFile )
{
    if ( QFile::exists( testFile ) )
        QFile::remove( testFile );
    QDir dirTest( testFile );
    if ( dirTest.exists() ) {
        system( QByteArray( "rm -rf " ) + QFile::encodeName( testFile ) ); // QDir::rmdir isn't recursive! (EEEEK! system! *cries*)
    }

    kDebug() << "======================="<<testName<<"====================================" << endl;
    KoStore* store = KoStore::createStore( testFile, KoStore::Write, "", backend );
    if ( store->bad() )
        return cleanUp( store, testFile, badStorage );

    // Write
    if ( store->open( "layer" ) ) {
        char str[DATALEN];

        sprintf(str, "1,2,3,4\n");
        store->write(str,strlen(str));
        memset(str, '\0', DATALEN);
        store->write(str, DATALEN);

        store->close();
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    if ( store->isOpen() )
        store->close();
    delete store;

    kDebug() << "===========================================================" << endl;

    store = KoStore::createStore( testFile, KoStore::Read, "", backend );
    if ( store->bad() )
        return cleanUp( store, testFile, badStorage );

    // Read back
    if ( store->open( "layer" ) ) {
        char str[DATALEN];
        QIODevice *stream = store->device(); // << Possible suspect!

        stream->readLine(str, DATALEN);      // << as is this
        qint64 len = store->read(str, DATALEN);
        if (len != DATALEN) {
            kDebug() << "Final length was too small: " << len << " < " << DATALEN << endl;
            return 1;
        }

        store->close();
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    if ( store->isOpen() )
        store->close();
    delete store;

    QFile::remove( testFile );

    kDebug() << "===========================================================" << endl;
    return 0;
}

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "storage_test2", "Storage Test 2", "A test for the KoStore classes, in particular a Krita-induced bug testcase", "1" );
    //KApplication::disableAutoDcopRegistration();
    KApplication app(false);

    if ( test( "Tar", KoStore::Tar, "test.tgz" ) != 0 )
      return 1;
    if ( test( "Directory", KoStore::Directory, "testdir/maindoc.xml" ) != 0 )
      return 1;
    if ( test( "Zip", KoStore::Zip, "test.zip" ) != 0 )
      return 1;
}
