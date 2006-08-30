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

#include "KoStoreBase.h"
#include <QHash>
#include <QtCrypto>

class QString;
class QByteArray;
class QIODevice;
class QWidget;
class KUrl;
class KoZipStore;
struct KoEncryptedStore_EncryptionData;

class KoEncryptedStore : public KoStoreBase
{
public:
    KoEncryptedStore( const QString & filename, Mode mode, const QByteArray & appIdentification );
    KoEncryptedStore( QIODevice *dev, Mode mode, const QByteArray & appIdentification );
    KoEncryptedStore( QWidget* window, const KUrl& url, const QString & filename, Mode _mode, const QByteArray & appIdentification );
    ~KoEncryptedStore();

    /**
     * Creates a KoStore to read a ZIP-file.
     *
     * This method will return a KoEncryptedStore if the file is encrypted or a KoZipStore if it's not.
     * It is advisable to use to function when trying to read a ZIP-file: this function contains the logic to determine if a ZIP
     * is encrypted and will give you the fastest KoStore to read the file that will actually work.
     *
     * The arguments are the same as in the constructors.
     *
     * @return  A store to read the file.
     */
    // Creating a KoEncryptedStore will give more overhead and a KoZipStore can't decrypt encrypted files
    static KoStore* createEncryptedStoreReader( const QString & filename, const QByteArray & appIdentification );
    /**
     * Creates a KoStore to read a ZIP-file.
     *
     * This method will return a KoEncryptedStore if the file is encrypted or a KoZipStore if it's not.
     * It is advisable to use to function when trying to read a ZIP-file: this function contains the logic to determine if a ZIP
     * is encrypted and will give you the fastest KoStore to read the file that will actually work.
     *
     * The arguments are the same as in the constructors.
     *
     * @return  A store to read the device.
     */
    // Creating a KoEncryptedStore will give more overhead and a KoZipStore can't decrypt encrypted files
    static KoStore* createEncryptedStoreReader( QIODevice *dev, const QByteArray & appIdentification );

    /**
     * Creates a KoStore to read a ZIP-file.
     *
     * This method will return a KoEncryptedStore if the file is encrypted or a KoZipStore if it's not.
     * It is advisable to use to function when trying to read a ZIP-file: this function contains the logic to determine if a ZIP
     * is encrypted and will give you the fastest KoStore to read the file that will actually work.
     *
     * The arguments are the same as in the constructors.
     *
     * @return  A store to read the url.
     */
    // Creating a KoEncryptedStore will give more overhead and a KoZipStore can't decrypt encrypted files
    static KoStore* createEncryptedStoreReader( QWidget* window, const KUrl& url, const QString & filename, const QByteArray & appIdentification );

    /**
     * Sets the password to be used for decryption or encryption of the file.
     *
     * This method only works if no password has been set or found yet,
     * i.e. when no file has been opened yet and this method hasn't been used yet.
     *
     * @param   password    A non-empty password.
     *
     * @return  True if the password was set
     */
    bool setPassword( const QString& password );

protected:
    virtual bool init( Mode mode, const QByteArray& appIdentification );
    virtual bool openWrite( const QString& name );
    virtual bool openRead( const QString& name );
    virtual bool closeWrite();
    virtual bool closeRead();
    virtual bool enterRelativeDirectory( const QString& dirName );
    virtual bool enterAbsoluteDirectory( const QString& path );
    virtual bool fileExists( const QString& absPath ) const;

    /**
     * Returns whether a store opened for reading is actually encrypted.
     * This function will always return true in Write-mode.
     *
     * @return  True if the store is encrypted.
     */
    bool isEncrypted( );

    /**
     * Internal function.
     * Rips the zip-store that drives the encrypted store from it. The encrypted store won't work anymore after this.
     * Never use this function once a writing action has begun: the encryption-data will be lost and so will all data that
     * has been written.
     *
     * @return  The KoZipStore driving this store.
     */
    KoZipStore *ripZipStore( );

    /**
     * Tries and find a password for this document in KWallet.
     * Uses m_filename as base for finding the password and stores it in m_password if found.
     */
    void findPasswordInKWallet( );

    /**
     * Stores the password for this document in KWallet.
     * Uses m_filename as base for storing the password and stores the value in m_password.
     */
    void savePasswordInKWallet( );

private:
    QSecureArray decryptFile( QSecureArray & encryptedFile, KoEncryptedStore_EncryptionData & encData, QSecureArray & password );
    QString normalizedFullPath( const QString& fullpath );
    QString filenameOnly( const QString& fullpath );
    bool initBackend( );
    /* Deferred loading is used to prevent dataloss because the backend store would have opened the file,
     * but the user won't provide a password. The backend store is not loaded until a password is provided,
     * so files are not overwritten until they *can* be overwritten.
     */
    // Temporary store for a possible url, used for deferred loading of the backend store
    KUrl m_init_url;
    // Temporary store for a possible device, used for deferred loading of the backend store
    QIODevice *m_init_dev;
    // A flag used for deferred loading of the backend store
    bool m_init_deferred;
    // Temporary store for the application identification, used for deferred loading of the backend store
    QByteArray m_init_appIdentification;

protected:
    QCA::Initializer m_qcaInit;
    QHash<QString, KoEncryptedStore_EncryptionData> m_encryptionData;
    KoZipStore *m_store;
    QSecureArray m_password;
    QWidget *m_window;
    QString m_filename;
    QByteArray m_manifestBuffer;
};
