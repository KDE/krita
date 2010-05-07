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

#ifndef KoEncryptedStore_h
#define KoEncryptedStore_h

#include "KoStore.h"
#include <QHash>
#include <QtCrypto>

class QString;
class QByteArray;
class QIODevice;
class QWidget;
class KUrl;
class KZip;
class KArchiveDirectory;
class KTemporaryFile;
struct KoEncryptedStore_EncryptionData;

class KoEncryptedStore : public KoStore
{
public:
    KoEncryptedStore(const QString &filename, Mode mode, const QByteArray &appIdentification);
    KoEncryptedStore(QIODevice *dev, Mode mode, const QByteArray &appIdentification);
    KoEncryptedStore(QWidget *window, const KUrl &url, const QString &filename, Mode mode, const QByteArray &appIdentification);
    ~KoEncryptedStore();

    /*
     * Sets the password to be used for decryption or encryption of the file.
     *
     * This method only works if no password has been set or found yet,
     * i.e. when no file has been opened yet and this method hasn't been used yet.
     *
     * @param   password    A non-empty password.
     *
     * @return  True if the password was set
     */
    virtual bool setPassword(const QString &password);

    /*
     * Returns whether a store opened for reading is actually encrypted.
     * This function will always return true in Write-mode.
     *
     * @return  True if the store is encrypted.
     */
    virtual bool isEncrypted();

protected:

    using KoStore::init;

    virtual bool init(Mode mode, const QByteArray &appIdentification);
    virtual bool doFinalize();
    virtual bool openWrite(const QString &name);
    virtual bool openRead(const QString &name);
    virtual bool closeWrite();
    virtual bool closeRead();
    virtual bool enterRelativeDirectory(const QString &dirName);
    virtual bool enterAbsoluteDirectory(const QString &path);
    virtual bool fileExists(const QString &absPath) const;

    /**
     * Tries and find a password for this document in KWallet.
     * Uses m_filename as base for finding the password and stores it in m_password if found.
     */
    void findPasswordInKWallet();

    /*
     * Retrieves the password used to encrypt or decrypt the store. Note that
     * QString() will returned if no password has been given or the store is
     * not encrypted.
     *
     * @return  The password this store is encrypted with.
     */
    virtual QString password();

    /**
     * Stores the password for this document in KWallet.
     * Uses m_filename as base for storing the password and stores the value in m_password.
     */
    void savePasswordInKWallet();

private:
    QCA::SecureArray decryptFile(QCA::SecureArray & encryptedFile, KoEncryptedStore_EncryptionData & encData, QCA::SecureArray & password);

    /** returns true if the file should be encrypted, false otherwise **/
    bool isToBeEncrypted(const QString &fullpath);

protected:
    QCA::Initializer m_qcaInit;
    QHash<QString, KoEncryptedStore_EncryptionData> m_encryptionData;
    QCA::SecureArray m_password;
    QString m_filename;
    QByteArray m_manifestBuffer;
    KZip *m_pZip;
    KTemporaryFile *m_tempFile;
    bool m_bPasswordUsed;
    bool m_bPasswordDeclined;

    /** In "Read" mode this pointer is pointing to the
    current directory in the archive to speed up the verification process */
    const KArchiveDirectory *m_currentDir;
private:
    Q_DECLARE_PRIVATE(KoStore)
};

#endif
