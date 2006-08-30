/* This file is part of the KDE project
   Copyright (C) 2006 Thomas Schaap <thomas.schaap@kdemail.net>

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

#include "KoXmlWriter.h"
#include <KoXmlReader.h>
#include "KoZipStore.h"
#include <QString>
#include <QByteArray>
#include <QIODevice>
#include <QWidget>
#include <QtCrypto>
#include <kpassworddialog.h>
#include "KoEncryptedStore.h"
#include <kwallet.h>
#include <klocale.h>
#include <kfilterdev.h>
#include <kmessage.h>
#include <kmessagebox.h>

struct KoEncryptedStore_EncryptionData {
    // Needed for Key Derivation
    QSecureArray salt;
    unsigned int iterationCount;

    // Needed for enc/decryption
    QSecureArray initVector;

    // Needed for (optional) password-checking
    QSecureArray checksum;
    // checksumShort is set to true if the checksum-algorithm is SHA1/1K, which basically means we only use the first 1024 bytes of the unencrypted file to check against (see also http://www.openoffice.org/servlets/ReadMsg?list=dev&msgNo=17498)
    bool checksumShort;

    // The size of the uncompressed file
    qint64 filesize;
};

namespace {
    const char* MANIFEST_FILE = "META-INF/manifest.xml";
    const char* META_FILE = "meta.xml";
    const char* THUMBNAIL_FILE = "Thumbnails/thumbnail.png";
}

KoEncryptedStore::KoEncryptedStore( const QString & filename, Mode mode, const QByteArray & appIdentification )
    : m_init_url( NULL ), m_init_dev( NULL ), m_init_deferred( false ), m_init_appIdentification( appIdentification ), m_qcaInit( QCA::Initializer() ), m_password( QSecureArray() ), m_window( NULL ), m_filename( QString( filename ) ), m_manifestBuffer( QByteArray() ) {
    if( mode == Write ) {
        // Prevent the underlying store from being opened if encryption is not supported
        m_bGood = QCA::isSupported( "sha1" ) && QCA::isSupported( "pbkdf2(sha1)" ) && init( mode, appIdentification );
        m_store = NULL;
        m_init_deferred = true;
        return;
    }
    m_store = new KoZipStore( filename, mode, appIdentification );
    m_bGood = m_store && !m_store->bad( );

    if( m_bGood ) {
        m_bGood = init( mode, appIdentification );
    }
}

KoEncryptedStore::KoEncryptedStore( QIODevice *dev, Mode mode, const QByteArray & appIdentification )
    : m_init_url( NULL ), m_init_dev( dev ), m_init_deferred( false ), m_init_appIdentification( appIdentification ), m_qcaInit( QCA::Initializer() ), m_password( QSecureArray() ), m_window( NULL ), m_filename( QString( ) ), m_manifestBuffer( QByteArray() ) {
    if( mode == Write ) {
        // Prevent the underlying store from being opened if encryption is not supported
        m_bGood = QCA::isSupported( "sha1" ) && QCA::isSupported( "pbkdf2(sha1)" ) && init( mode, appIdentification );
        m_store = NULL;
        m_init_deferred = true;
        return;
    }
    m_store = new KoZipStore( dev, mode, appIdentification );
    m_bGood = m_store && !m_store->bad( );

    if( m_bGood ) {
        m_bGood = init( mode, appIdentification );
    }
}

KoEncryptedStore::KoEncryptedStore( QWidget* window, const KUrl& url, const QString & filename, Mode mode, const QByteArray & appIdentification )
    : m_init_url( url ), m_init_dev( NULL ), m_init_deferred( false ), m_init_appIdentification( appIdentification ), m_qcaInit( QCA::Initializer() ), m_password( QSecureArray() ), m_window( window ), m_filename( QString( url.toString( ) ) ), m_manifestBuffer( QByteArray() ) {
    if( mode == Write ) {
        // Prevent the underlying store from being opened if encryption is not supported
        m_bGood = QCA::isSupported( "sha1" ) && QCA::isSupported( "pbkdf2(sha1)" ) && init( mode, appIdentification );
        m_store = NULL;
        m_init_deferred = true;
        return;
    }
    m_store = new KoZipStore( window, url, filename, mode, appIdentification );
    m_bGood = m_store && !m_store->bad( );

    if( m_bGood ) {
        m_bGood = init( mode, appIdentification );
    }
}

KoEncryptedStore::~KoEncryptedStore() {
    if( isOpen( ) ) {
        close( );
    }
    if( m_store ) {
        if( m_store->isOpen( ) ) {
            m_store->close( );
        }
        if( m_mode == Write ) {
            // First change the manifest file and write it
            // We'll use the QDom classes here, since KoXmlReader and KoXmlWriter have no way of copying a complete xml-file
            // other than parsing it completely and rebuilding it.
            // Errorhandling here is done to prevent data from being lost whatever happens
            QDomDocument document;
            if( m_manifestBuffer.isEmpty( ) ) {
                // No manifest? Better create one
                document = QDomDocument::QDomDocument( );
                QDomElement rootElement = document.createElement( "manifest:manifest" );
                rootElement.setAttribute( "xmlns:manifest", "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0" );
                document.appendChild( rootElement );
            }
            if( !m_manifestBuffer.isEmpty( ) && !document.setContent( m_manifestBuffer ) ) {
                // Oi! That's fresh XML we should have here!
                // This is the only case we can't fix
                KMessage::message( KMessage::Error, i18n( "The manifest file seems to be corrupted. It can not be modified and the document will remain unreadable. Please try and save the document again to prevent losing your work." ) );
            }
            else {
                QDomElement documentElement = document.documentElement( );
                QDomNodeList fileElements = documentElement.elementsByTagName( "manifest:file-entry" );
                // Search all files in the manifest
                QStringList foundFiles;
                for( int i = 0; i < fileElements.size( ); i++ ) {
                    printf( "Tussen, nummertje %d:\n\n%s\n\n", i, document.toByteArray( ).data( ) );
                    QDomElement fileElement = fileElements.item( i ).toElement( );
                    QString fullpath = fileElement.toElement( ).attribute( "manifest:full-path" );
                    // See if it's encrypted
                    if( fullpath.isEmpty( ) || !m_encryptionData.contains( normalizedFullPath( fullpath ) ) ) {
                        continue;
                    }
                    foundFiles += normalizedFullPath( fullpath );
                    KoEncryptedStore_EncryptionData encData = m_encryptionData.value( normalizedFullPath( fullpath ) );
                    // Set the unencrypted size of the file
                    fileElement.setAttribute( "manifest:size", encData.filesize );
                    // See if the user of this store has already provided (old) encryption data
                    QDomNodeList childElements = fileElement.elementsByTagName( "manifest:encryption-data" );
                    QDomElement encryptionElement;
                    QDomElement algorithmElement;
                    QDomElement keyDerivationElement;
                    if( childElements.isEmpty( ) ) { 
                        encryptionElement = document.createElement( "manifest:encryption-data" );
                        fileElement.appendChild( encryptionElement );
                    }
                    else {
                        encryptionElement = childElements.item( 0 ).toElement( );
                    }
                    childElements = encryptionElement.elementsByTagName( "manifest:algorithm" );
                    if( childElements.isEmpty( ) ) {
                        algorithmElement = document.createElement( "manifest:algorithm" );
                        encryptionElement.appendChild( algorithmElement );
                    }
                    else {
                        algorithmElement = childElements.item( 0 ).toElement( );
                    }
                    childElements = encryptionElement.elementsByTagName( "manifest:key-derivation" );
                    if( childElements.isEmpty( ) ) {
                        keyDerivationElement = document.createElement( "manifest:key-derivation" );
                        encryptionElement.appendChild( keyDerivationElement );
                    }
                    else {
                        keyDerivationElement = childElements.item( 0 ).toElement( );
                    }
                    // Set the right encryption data
                    QCA::Base64 encoder;
                    QSecureArray checksum = encoder.update( encData.checksum );
                    checksum += encoder.final( );
                //    encryptionElement.setAttribute( "manifest:checksum", QString( checksum.toByteArray( ) ) );
                    if( encData.checksumShort ) {
                  //      encryptionElement.setAttribute( "manifest:checksum-type", "SHA1/1K" );
                    }
                    else {
                    //    encryptionElement.setAttribute( "manifest:checksum-type", "SHA1" );
                    }
                    encoder.clear( );
                    QSecureArray initVector = encoder.update( encData.initVector );
                    initVector += encoder.final( );
                    algorithmElement.setAttribute( "manifest:initialisation-vector", QString( initVector.toByteArray( ) ) );
                    algorithmElement.setAttribute( "manifest:algorithm-name", "Blowfish CFB" );
                    encoder.clear( );
                    QSecureArray salt = encoder.update( encData.salt );
                    salt += encoder.final( );
                    keyDerivationElement.setAttribute( "manifest:salt", QString( salt.toByteArray( ) ) );
                    keyDerivationElement.setAttribute( "manifest:key-derivation-name", "PBKDF2" );
                }
                if( foundFiles.size( ) < m_encryptionData.size( ) ) {
                    QList<QString> keys = m_encryptionData.keys( );
                    for( int i = 0; i < keys.size( ); i++ ) {
                        if( !foundFiles.contains( normalizedFullPath( keys.value( i ) ) ) ) {
                            KoEncryptedStore_EncryptionData encData = m_encryptionData.value( normalizedFullPath( keys.value( i ) ) );
                            QDomElement fileElement = document.createElement( "manifest:file-entry" );
                            fileElement.setAttribute( "manifest:full-path", normalizedFullPath( keys.value( i ) ).mid( 1 ) );
                            fileElement.setAttribute( "manifest:size", encData.filesize );
                            fileElement.setAttribute( "manifest:media-type", "" );
                            documentElement.appendChild( fileElement );
                            QDomElement encryptionElement = document.createElement( "manifest:encryption-data" );
                            QCA::Base64 encoder;
                            QSecureArray checksum = encoder.update( encData.checksum );
                            checksum += encoder.final( );
                            encoder.clear( );
                            QSecureArray initVector = encoder.update( encData.initVector );
                            initVector += encoder.final( );
                            encoder.clear( );
                            QSecureArray salt = encoder.update( encData.salt );
                            salt += encoder.final( );
                            encryptionElement.setAttribute( "manifest:checksum", QString( checksum.toByteArray( ) ) );
                            if( encData.checksumShort ) {
                                encryptionElement.setAttribute( "manifest:checksum-type", "SHA1/1K" );
                            }
                            else {
                                encryptionElement.setAttribute( "manifest:checksum-type", "SHA1" );
                            }
                            fileElement.appendChild( encryptionElement );
                            QDomElement algorithmElement = document.createElement( "manifest:algorithm" );
                            algorithmElement.setAttribute( "manifest:algorithm-name", "Blowfish CFB" );
                            algorithmElement.setAttribute( "manifest:initialisation-vector", QString( initVector.toByteArray( ) ) );
                            encryptionElement.appendChild( algorithmElement );
                            QDomElement keyDerivationElement = document.createElement( "manifest:key-derivation" );
                            keyDerivationElement.setAttribute( "manifest:key-derivation-name", "PBKDF2" );
                            keyDerivationElement.setAttribute( "manifest:salt", QString( salt.toByteArray( ) ) );
                            encryptionElement.appendChild( keyDerivationElement );
                        }
                    }
                }
                printf( "Voor:\n\n%s\n", m_manifestBuffer.data( ) );
                m_manifestBuffer = document.toByteArray( );
                printf( "\nNa:\n\n%s\n\n", m_manifestBuffer.data( ) );
                if( !m_store->open( MANIFEST_FILE ) ) {
                    KMessage::message( KMessage::Error, i18n( "The manifest file can't be opened. The document will remain unreadable. Please try and save the document again to prevent losing your work." ) );
                }
                else {
                    if( static_cast<KoStore *>( m_store )->write( m_manifestBuffer ) != m_manifestBuffer.size( ) ) {
                        KMessage::message( KMessage::Error, i18n( "The manifest file can't be opened. The document will remain unreadable. Please try and save the document again to prevent losing your work." ) );
                    }
                    m_store->close( );
                }
            }
        }

        delete m_store;
    }
}

KoZipStore* KoEncryptedStore::ripZipStore( ) {
    if( !initBackend( ) ) {
        return NULL;
    }
    if( m_store ) {
        KoZipStore *store = m_store;
        m_store = NULL;
        return store;
    }
    return NULL;
}

bool KoEncryptedStore::initBackend( ) {
    if( m_init_deferred ) {
        if( m_init_dev ) {
            m_store = new KoZipStore( m_init_dev, m_mode, m_init_appIdentification );
            m_init_dev = NULL;
        }
        else if( !m_init_url.isEmpty( ) ) {
            m_store = new KoZipStore( m_window, m_init_url, m_filename, m_mode, m_init_appIdentification );
            m_init_url = KUrl( );
        }
        else {
            m_store = new KoZipStore( m_filename, m_mode, m_init_appIdentification );
        }
        m_init_deferred = false;
        if( !m_store || m_store->bad( ) ) {
            m_bGood = false;
            if( isOpen( ) ) {
                close( );
            }
            return false;
        }
        if( isOpen( ) ) {
            if( !m_store->open( normalizedFullPath( m_sName ) ) ) {
                m_bGood = false;
                close( );
                return false;
            }
        }
        m_init_deferred = false;
    }
    return true;
}

KoStore* KoEncryptedStore::createEncryptedStoreReader( const QString & filename, const QByteArray & appIdentification ) {
    KoEncryptedStore *encStore = new KoEncryptedStore( filename, Read, appIdentification );
    if( encStore->isEncrypted( ) ) {
        return encStore;
    }
    KoZipStore *zipStore = encStore->ripZipStore( );
    delete encStore;
    return zipStore;
}

KoStore* KoEncryptedStore::createEncryptedStoreReader( QIODevice *dev, const QByteArray & appIdentification ) {
    KoEncryptedStore *encStore = new KoEncryptedStore( dev, Read, appIdentification );
    if( encStore->isEncrypted( ) ) {
        return encStore;
    }
    KoZipStore *zipStore = encStore->ripZipStore( );
    delete encStore;
    return zipStore;
}

KoStore* KoEncryptedStore::createEncryptedStoreReader( QWidget* window, const KUrl& url, const QString & filename, const QByteArray & appIdentification ) {
    KoEncryptedStore *encStore = new KoEncryptedStore( window, url, filename, Read, appIdentification );
    if( encStore->isEncrypted( ) ) {
        return encStore;
    }
    KoZipStore *zipStore = encStore->ripZipStore( );
    delete encStore;
    return zipStore;
}

bool KoEncryptedStore::init( Mode mode, const QByteArray& /*appIdentification*/ ) {
    KoStore::init( mode );
    m_mode = mode;
    if( mode == Read ) {
        // Read the manifest-file, so we can get the data out we'll need to decrypt the other files in the store
        m_store->pushDirectory( );
        bool ok = m_store->open( "tar:/META-INF/manifest.xml" );
        if( !ok ) {
            if( !m_store->bad( ) && !m_store->hasFile( "tar:/META-INF/manifest.xml" ) ) {
                return true; // No manifest file? OK, *I* won't complain.
            }
            m_bGood = false;
            return false;
        }
        QIODevice *dev = m_store->device( );

        KoXmlDocument xmldoc;
        if( !xmldoc.setContent( dev ) ) {
            KMessage::message( KMessage::Warning, i18n( "The manifest file seems to be corrupted. The document could not be opened." ) );
            m_store->close( );
            m_bGood = false;
            return false;
        }
        KoXmlElement xmlroot = xmldoc.documentElement( );
        if( xmlroot.tagName( ) != "manifest:manifest" ) {
            KMessage::message( KMessage::Warning, i18n( "The manifest file seems to be corrupted. The document could not be opened." ) );
            m_store->close( );
            m_bGood = false;
            return false;
        }

        if( xmlroot.hasChildNodes( ) ) {
            QCA::Base64 base64decoder( QCA::Decode );
            KoXmlNode xmlnode = xmlroot.firstChild( );
            while( !xmlnode.isNull( ) ) {
                // Search for files
                if( !xmlnode.isElement( ) || xmlnode.toElement( ).tagName( ) != "manifest:file-entry" || !xmlnode.hasChildNodes( ) || !xmlnode.toElement( ).hasAttribute( "manifest:full-path" ) ) {
                    xmlnode = xmlnode.nextSibling( );
                    continue;
                }

                // Build a structure to hold the data and fill it with defaults
                KoEncryptedStore_EncryptionData encData;
                encData.filesize = 0;
                encData.checksum = QSecureArray();
                encData.checksumShort = false;
                encData.salt = QSecureArray();
                encData.iterationCount = 0;
                encData.initVector = QSecureArray();

                // Get some info about the file
                QString fullpath = xmlnode.toElement( ).attribute( "manifest:full-path" );
                if( xmlnode.toElement( ).hasAttribute( "manifest:size" ) ) {
                    encData.filesize = xmlnode.toElement( ).attribute( "manifest:size" ).toUInt( );
                }

                // Find the embedded encryption-data block
                KoXmlNode xmlencnode = xmlnode.firstChild( );
                while( !xmlencnode.isNull( ) && ( !xmlencnode.isElement( ) || xmlencnode.toElement( ).tagName( ) != "manifest:encryption-data" || !xmlencnode.hasChildNodes( ) ) ) {
                    xmlencnode = xmlencnode.nextSibling( );
                }
                if( xmlencnode.isNull( ) ) {
                    xmlnode = xmlnode.nextSibling( );
                    continue;
                }

                // Find some things about the checksum
                if( xmlencnode.toElement( ).hasAttribute( "manifest:checksum" ) ) {
                    base64decoder.clear( );
                    encData.checksum = base64decoder.update( QSecureArray( xmlencnode.toElement( ).attribute( "manifest:checksum" ).toAscii( ) ) );
                    encData.checksum += base64decoder.final( );
                    if( xmlencnode.toElement( ).hasAttribute( "manifest:checksum-type" ) ) {
                        QString checksumType = xmlencnode.toElement( ).attribute( "manifest:checksum-type" );
                        if( checksumType == "SHA1" ) {
                            encData.checksumShort = false;
                        }
                        // For this particual hash-type: check KoEncryptedStore_encryptionData.checksumShort
                        else if( checksumType == "SHA1/1K" ) {
                            encData.checksumShort = true;
                        }
                        else {
                            // Checksum type unknown
                            KMessage::message( KMessage::Warning, i18n( "This document contains an unknown checksum. When you give a password it might not be verified." ) );
                            encData.checksum = QSecureArray();
                        }
                    }
                    else {
                        encData.checksumShort = false;
                    }
                }

                KoXmlNode xmlencattr = xmlencnode.firstChild( );
                bool algorithmFound = false;
                bool keyDerivationFound = false;
                // Search all data about encrption
                while( !xmlencattr.isNull( ) ) {
                    if( !xmlencattr.isElement( ) ) {
                        xmlencattr = xmlencattr.nextSibling( );
                        continue;
                    }
                    
                    // Find some things about the encryption algorithm
                    if( xmlencattr.toElement( ).tagName( ) == "manifest:algorithm" && xmlencattr.toElement( ).hasAttribute( "manifest:initialisation-vector" ) ) {
                        algorithmFound = true;
                        base64decoder.clear( );
                        encData.initVector = base64decoder.update( QSecureArray( xmlencattr.toElement( ).attribute( "manifest:initialisation-vector" ).toAscii( ) ) );
                        encData.initVector += base64decoder.final( );
                        if( xmlencattr.toElement( ).hasAttribute( "manifest:algorithm-name" ) && xmlencattr.toElement( ).attribute( "manifest:algorithm-name" ) != "Blowfish CFB" ) {
                            KMessage::message( KMessage::Warning, i18n( "This document contains an unknown encryption method. Some parts may be unreadable." ) );
                            encData.initVector = QSecureArray();
                        }
                    }

                    // Find some things about the key derivation
                    if( xmlencattr.toElement( ).tagName( ) == "manifest:key-derivation" && xmlencattr.toElement( ).hasAttribute( "manifest:salt" ) ) {
                        keyDerivationFound = true;
                        base64decoder.clear( );
                        encData.salt = base64decoder.update( QSecureArray( xmlencattr.toElement( ).attribute( "manifest:salt" ).toAscii( ) ) );
                        encData.salt += base64decoder.final( );
                        encData.iterationCount = 1024;
                        if( xmlencattr.toElement( ).hasAttribute( "manifest:iteration-count" ) ) {
                            encData.iterationCount = xmlencattr.toElement( ).attribute( "manifest:iteration-count" ).toUInt( );
                        }
                        if( xmlencattr.toElement( ).hasAttribute( "manifest:key-derivation-name" ) && xmlencattr.toElement( ).attribute( "manifest:key-derivation-name" ) != "PBKDF2" ) {
                            KMessage::message( KMessage::Warning, i18n( "This document contains an unknown encryption method. Some parts may be unreadable." ) );
                            encData.salt = QSecureArray();
                        }
                    }
                    
                    xmlencattr = xmlencattr.nextSibling( );
                }

                // Only use this encryption data if it makes sense to use it
                if( !( encData.salt.isEmpty() || encData.initVector.isEmpty() ) ) {
                    m_encryptionData.insert( normalizedFullPath( fullpath ), encData );
                    if( !( algorithmFound && keyDerivationFound ) ) {
                        KMessage::message( KMessage::Warning, i18n( "This document contains incomplete encryption data. Some parts may be unreadable." ) );
                    }
                }

                xmlnode = xmlnode.nextSibling( );
            }
        }

        // Let's make sure we're clean at the beginning again: noone needs to know we've been nosing around
        m_store->close( );
        m_store->popDirectory( );
    }
    
    return true;
}

bool KoEncryptedStore::isEncrypted( ) {
    if( m_mode == Read ) {
        return !m_encryptionData.isEmpty( );
    }
    return true;
}

bool KoEncryptedStore::openWrite( const QString& name ) {
    if( normalizedFullPath( name ) != MANIFEST_FILE ) {
        if( !m_store && !m_init_deferred ) {
            return false;
        }
        if( m_store && !m_store->open( normalizedFullPath( name ) ) ) {
            return false;
        }
    }
    m_stream = new QBuffer( );
    if( !m_stream->open( QIODevice::WriteOnly ) ) {
        return false;
    }
    return true;
}

bool KoEncryptedStore::openRead( const QString& name ) {
    if( !m_store || !m_store->open( name ) ) {
        return false;
    }
    if( !m_encryptionData.contains( normalizedFullPath( name ) ) ) {
        // The file is not encrypted, or at least we don't know about it, simply pass on
        if( m_stream ) {
            delete m_stream;
        }
        m_stream = m_store->device( );
        m_iSize = m_store->size( );
    }
    else {
        QSecureArray encryptedFile( m_store->device( )->readAll( ) );
        if( encryptedFile.size( ) != m_store->size( ) ) {
            // Read error detected
            m_store->close( );
            return false;
        }
        m_store->close( );
        KoEncryptedStore_EncryptionData encData = m_encryptionData.value( normalizedFullPath( name ) );
        QSecureArray decrypted;

        // If we don't have a password yet, try and find one
        if( m_password.isEmpty( ) ) {
            findPasswordInKWallet( );
        }

        // Used to be "while( passwordOK )", but why use a flag if I can just say "break" on that one place I set that flag?
        while( true ) {
            QByteArray pass;
            QSecureArray password;
            int keepPass = 0;
            // I already have a password! Let's try it. If it's not good, we can dump it, anyway.
            if( !m_password.isEmpty( ) ) {
                password = m_password;
                m_password = QSecureArray();
            }
            else {
                QByteArray pass;
                if( !m_filename.isNull( ) )
                    keepPass = 1;
                if( KPasswordDialog::getPassword( m_window, pass, i18n( "Please enter the password to open this file." ), &keepPass ) == KPasswordDialog::Rejected ) {
                    return false;
                }
                password = QSecureArray( pass );
                if( password.isEmpty( ) ) {
                    continue;
                }
            }

            decrypted = decryptFile( encryptedFile, encData, password );
            if( decrypted.isEmpty() ) {
                return false;
            }

            if( !encData.checksum.isEmpty() ) {
                QSecureArray checksum;
                if( encData.checksumShort && decrypted.size( ) > 1024 ) {
                    // TODO: Eww!!!! I don't want to convert via insecure arrays to get the first 1K characters of a secure array <- fix QCA?
                    checksum = QCA::Hash( "sha1" ).hash( QSecureArray( decrypted.toByteArray( ).left( 1024 ) ) );
                }
                else {
                    checksum = QCA::Hash( "sha1" ).hash( decrypted );
                }
                if( checksum != encData.checksum ) {
                    continue;
                }
            }

            // The password passed all possible tests, so let's accept it
            m_password = password;

            if( keepPass ) {
                savePasswordInKWallet( );
            }

            break;
        }

        QByteArray *resultArray = new QByteArray( decrypted.toByteArray( ) );
        QIODevice *resultDevice = KFilterDev::device( new QBuffer( resultArray, NULL ), "application/x-gzip" );
        if( !resultDevice ) {
            delete resultArray;
            return false;
        }
        static_cast<KFilterDev*>( resultDevice )->setSkipHeaders( );
        m_stream = resultDevice;
        m_iSize = encData.filesize;
    }

    return true;
}

void KoEncryptedStore::findPasswordInKWallet( ) {
    /* About KWallet access
     *
     * The choice has been made to postfix every entry in a kwallet concerning passwords for opendocument files with /opendocument
     * This choice has been made since, at the time of this writing, the auhor could not find any reference as to standardised
     * naming schemes for entries in the wallet. Since collission of passwords in entries should be avoided and is at least possible,
     * considering remote files might be both protected by a secured web-area (konqueror makes an entry) and a password (we make an
     * entry), it seems a good thing to make sure it won't happen.
     */
    if( !m_filename.isNull( ) && !KWallet::Wallet::folderDoesNotExist( KWallet::Wallet::LocalWallet( ), KWallet::Wallet::PasswordFolder( ) ) && !KWallet::Wallet::keyDoesNotExist( KWallet::Wallet::LocalWallet( ), KWallet::Wallet::PasswordFolder( ), m_filename + "/opendocument" ) ) {
        KWallet::Wallet *wallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet( ), m_window ? m_window->winId( ) : 0 );
        if( wallet ) {
            if( wallet->setFolder( KWallet::Wallet::PasswordFolder( ) ) ) {
                QString pass;
                wallet->readPassword( m_filename + "/opendocument", pass );
                m_password = QSecureArray( pass.toUtf8( ) );
            }
            delete wallet;
        }
    }
}

void KoEncryptedStore::savePasswordInKWallet( ) {
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet( ), m_window ? m_window->winId( ) : 0 );
    if( wallet ) {
        if( !wallet->hasFolder( KWallet::Wallet::PasswordFolder( ) ) ) {
            wallet->createFolder( KWallet::Wallet::PasswordFolder( ) );
        }
        if( wallet->setFolder( KWallet::Wallet::PasswordFolder( ) ) ) {
            if( wallet->hasEntry( m_filename + "/opendocument" ) ) {
                wallet->removeEntry( m_filename + "/opendocument" );
            }
            wallet->writePassword( m_filename + "/opendocument", m_password.toByteArray( ).data( ) );
        }
        delete wallet;
    }
}

QSecureArray KoEncryptedStore::decryptFile( QSecureArray & encryptedFile, KoEncryptedStore_EncryptionData & encData, QSecureArray & password ) {
    if( !QCA::isSupported( "sha1" ) || !QCA::isSupported( "pbkdf2(sha1)" ) ) {
        return QSecureArray( );
    }
    QSecureArray keyhash = QCA::Hash( "sha1" ).hash( password );
    QCA::SymmetricKey key = QCA::PBKDF2( "sha1" ).makeKey( keyhash, QCA::InitializationVector( encData.salt ), 16, encData.iterationCount );
    QCA::Cipher decrypter( "blowfish", QCA::Cipher::CFB, QCA::Cipher::DefaultPadding, QCA::Decode, key, QCA::InitializationVector( encData.initVector ) );
    QSecureArray result = decrypter.update( encryptedFile );
    result += decrypter.final( );
    return result;
}

bool KoEncryptedStore::setPassword( const QString& password ) {
    if( !m_password.isEmpty() || password.isEmpty() ) {
        return false;
    }
    m_password = QSecureArray( password.toUtf8() );
    initBackend( );
    return true;
}

bool KoEncryptedStore::closeWrite() {
    bool passWasAsked = false;
    if( normalizedFullPath( m_sName ) == MANIFEST_FILE ) {
        m_manifestBuffer = static_cast<QBuffer*>( m_stream )->buffer( );
        return true;
    }
    // Allow close, but don't process it when nothing can be done in the backend
    if( !m_store || !m_store->isOpen( ) ) {
        return true;
    }

    // Find a password
    if( m_password.isEmpty() ) {
        findPasswordInKWallet( );
    }
    while( m_password.isEmpty( ) ) {
        QByteArray pass;
        if( KPasswordDialog::getNewPassword( m_window ? m_window : NULL, pass, i18n( "Please enter the password to encrypt the document with." ) ) == KPasswordDialog::Rejected ) {
            return false;
        }
        m_password = QSecureArray( pass );
        passWasAsked = true;
    }
    // So, we have a password, then we can initialize the backend (if necessary)
    if( !initBackend( ) ) {
        return false;
    }

    // Ask the user to save the password
    if( passWasAsked && KMessageBox::questionYesNo( m_window ? m_window : NULL, i18n( "Do you want to save the password?" ) ) == KMessageBox::Yes ) {
        savePasswordInKWallet( );
    }

    QByteArray resultData;
    if( normalizedFullPath( m_sName ) == META_FILE ) {
        // Save as-is
        resultData = static_cast<QBuffer*>( m_stream )->buffer( );
    }
    else if( normalizedFullPath( m_sName ) == THUMBNAIL_FILE ) {
        // TODO: Replace with a generic 'encrypted'-thumbnail
        resultData = static_cast<QBuffer*>( m_stream )->buffer( );
    }
    else {
        // Build all cryptographic data
        QSecureArray passwordHash = QCA::Hash( "sha1" ).hash( m_password );
        QCA::Random random;
        KoEncryptedStore_EncryptionData encData;
        encData.initVector = random.randomArray( 8, QCA::Random::LongTermKey );
        encData.salt = random.randomArray( 16, QCA::Random::LongTermKey );
        encData.iterationCount = 1024;
        QCA::SymmetricKey key = QCA::PBKDF2( "sha1" ).makeKey( passwordHash, QCA::InitializationVector( encData.salt ), 16, encData.iterationCount );
        QCA::Cipher encrypter( "blowfish", QCA::Cipher::CFB, QCA::Cipher::DefaultPadding, QCA::Encode, key, QCA::InitializationVector( encData.initVector ) );

        // Get the written data
        QByteArray data = static_cast<QBuffer*>( m_stream )->buffer( );
        encData.filesize = data.size( );

        // Compress the data
        QBuffer compressedData;
        QIODevice *compressDevice = KFilterDev::device( &compressedData, "application/x-gzip", false );
        if( !compressDevice) {
            return false;
        }
        static_cast<KFilterDev*>( compressDevice )->setSkipHeaders( );
        if( !compressDevice->open( QIODevice::WriteOnly ) ) {
            delete compressDevice;
            return false;
        }
        if( compressDevice->write( data ) != data.size( ) ) {
            delete compressDevice;
            return false;
        }
        compressDevice->close( );
        delete compressDevice;

        // Take a checksum of the data
        // Use the SHA1/1K method until it's clear if OOo supports plain SHA1
        // TODO: Find that out!
        if( data.size( ) > 1024 ) {
            QByteArray datatmp = compressedData.buffer( ).left( 1024 );
            encData.checksum = QCA::Hash( "sha1" ).hash( QSecureArray( datatmp ) );
        }
        else {
            encData.checksum = QCA::Hash( "sha1" ).hash( QSecureArray( compressedData.buffer( ) ) );
        }
        encData.checksumShort = true;

        // Encrypt the data
        QSecureArray result = encrypter.update( QSecureArray( compressedData.buffer( ) ) );
        result += encrypter.final( );
        resultData = result.toByteArray( );

        m_encryptionData.insert( normalizedFullPath( m_sName ), encData );
    }

    // Write it
    // TODO: Make sure it isn't compressed
    static_cast<KoStore *>( m_store )->write( resultData );
    
    return m_store->close( );
}

bool KoEncryptedStore::closeRead() {
    if( m_store && m_store->isOpen( ) ) {
        if( !m_store->close( ) ) {
            return false;
        }
        // m_stream is closed by m_store
        m_stream = NULL;
    }
    else if( m_stream ) {
        delete m_stream;
        m_stream = NULL;
    }
    return true;
}

// TODO: test these constructions
bool KoEncryptedStore::enterRelativeDirectory( const QString& dirName ) {
    if( m_store ) {
        m_store->pushDirectory( );
        bool res = m_store->enterDirectory( currentDirectory( ) ) && m_store->enterDirectory( dirName );
        m_store->popDirectory( );
        return res;
    }
    return m_init_deferred;
}

bool KoEncryptedStore::enterAbsoluteDirectory( const QString& path ) {
    if( m_store ) {
        m_store->pushDirectory( );
        bool res = m_store->enterDirectory( path );
        m_store->popDirectory( );
        return res;
    }
    return m_init_deferred;
}

bool KoEncryptedStore::fileExists( const QString& absPath ) const {
    if( m_mode == Write ) {
        return m_strFiles.contains( absPath );
    }
    if( m_store ) {
        int pos;
        QString tmp( absPath );

        // Clean path first
        if( tmp.left( 5 ) == "tar:/" )
            tmp = tmp.mid( 5 );
        if( tmp[0] == '/' )
            tmp = tmp.mid( 1 );

        m_store->pushDirectory( );
        if( !m_store->enterDirectory( "tar:/" ) ) {
            m_store->popDirectory( );
            return false;
        }

        while( ( pos = tmp.indexOf( '/' ) ) != -1 ) {
            if( !m_store->enterDirectory( tmp.left( pos ) ) ) {
                m_store->popDirectory( );
                return false;
            }
            tmp = tmp.mid( pos + 1 );
        }

        bool result = m_store->hasFile( absPath );
        m_store->popDirectory( );
        return result;
    }
    return false;
}

// Stupid little method to fix differences between internal paths and manifest:full-path paths
QString KoEncryptedStore::normalizedFullPath( const QString& fullpath ) {
    if( fullpath.startsWith( "/" ) ) {
        return fullpath.mid( 1 );
    }
    return fullpath;
}
