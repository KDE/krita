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

int cleanUp( KoStore* store, const QString& testFile, const char* error )
{
    QFile::remove( testFile );
    delete store;
    kDebug() << error << endl;
    return 1;
}

int test( const char* testName, KoStore::Backend backend, const QString& testFile )
{
    if ( QFile::exists( testFile ) )
        QFile::remove( testFile );
    QDir dirTest( testFile );
    if ( dirTest.exists() ) {
        system( QByteArray( "rm -rf " ) + QFile::encodeName( testFile ) ); // QDir::rmdir isn't recursive!
    }

    kDebug() << "======================="<<testName<<"====================================" << endl;
    KoStore* store = KoStore::createStore( testFile, KoStore::Write, "", backend );
    if ( store->bad() )
        return cleanUp( store, testFile, badStorage );

    if ( store->open( "test1/with/a/relative/dir.txt" ) ) {
        for ( int i = 0; i < 100; ++i )
            store->write( test1, strlen( test1 ) );
        store->close();
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    store->enterDirectory( testDir );
    if ( store->currentPath() != QString( testDirResult ) )
        return cleanUp( store, testFile, brokenPath );

    if ( store->open( "test2/with/a/relative/dir.txt" ) ) {
        for ( int i = 0; i < 100; ++i )
            store->write( test2, strlen( test2 ) );
        store->close();
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    if ( store->open( "root" ) ) {
        store->write( test3, strlen( test3 ) );
        store->close();
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    store->enterDirectory( testDir2 );
    if ( store->currentPath() != QString( testDir2Result ) )
        return cleanUp( store, testFile, brokenPath );

    if ( store->open( "root" ) ) {
        store->write( test4, strlen( test4 ) );
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

    if ( store->open( "test1/with/a/relative/dir.txt" ) ) {
        QIODevice* dev = store->device();
        int i = 0,  lim = strlen( test1 ),  count = 0;
        while ( static_cast<char>( dev->getch() ) == test1[i++] ) {
            if ( i == lim ) {
                i = 0;
                ++count;
            }
        }
        store->close();
        if ( count != 100 )
            return cleanUp( store, testFile, unableToRead );
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    store->enterDirectory( testDir );
    if ( store->currentPath() != QString( testDirResult ) )
        return cleanUp( store, testFile, brokenPath );

    if ( store->open( "test2/with/a/relative/dir.txt" ) ) {
        QIODevice* dev = store->device();
        int i = 0,  lim = strlen( test2 ),  count = 0;
        while ( static_cast<char>( dev->getch() ) == test2[i++] ) {
            if ( i == lim ) {
                i = 0;
                ++count;
            }
        }
        store->close();
        if ( count != 100 )
            return cleanUp( store, testFile, unableToRead );
    }
    else
        return cleanUp( store, testFile, unableToOpen );

    store->enterDirectory( testDir2 );
    store->pushDirectory();

    while ( store->leaveDirectory() );
    store->enterDirectory( testDir );
    if ( store->currentPath() != QString( testDirResult ) )
        return cleanUp( store, testFile, brokenPath );

    if ( store->open( "root" ) ) {
        if ( store->size() == 22 ) {
            QIODevice* dev = store->device();
            unsigned int i = 0;
            while ( static_cast<char>( dev->getch() ) == test3[i++] );
            store->close();
            if ( ( i - 1 ) != strlen( test3 ) )
                return cleanUp( store, testFile, unableToRead );
        }
        else {
            kError() << "Wrong size! maindoc.xml is " << store->size() << " should be 22." << endl;
            delete store;
            return 1;
        }
    }
    else {
        kError() << "Couldn't open storage!" << endl;
        delete store;
        return 1;
    }

    store->popDirectory();
    if ( store->currentPath() != QString( testDir2Result ) )
        return cleanUp( store, testFile, brokenPath );

    if ( store->open( "root" ) ) {
        char buf[29];
        store->read( buf, 28 );
        buf[28] = '\0';
        store->close();
        if ( strncmp( buf, test4, 28 ) != 0 )
            return cleanUp( store, testFile, unableToRead );
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
    KCmdLineArgs::init( argc, argv, "storage_test", "Storage Test", "A test for the KoStore classes", "1" );
    KApplication::disableAutoDcopRegistration();
    KApplication app;

    // KZip (due to KSaveFile) doesn't support relative filenames
    // So use $PWD as base for the paths explicitely.
    const QString testDir = QDir::currentPath();
    if ( test( "Tar", KoStore::Tar, testDir+"test.tgz" ) != 0 )
      return 1;
    if ( test( "Directory", KoStore::Directory, testDir+"testdir/maindoc.xml" ) != 0 )
      return 1;
    if ( test( "Zip", KoStore::Zip, testDir+"test.zip" ) != 0 )
      return 1;
}
