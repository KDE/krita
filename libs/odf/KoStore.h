/* This file is part of the KDE project
   Copyright (C) 1998, 1999 David Faure <faure@kde.org>
   Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>

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

#ifndef __koStore_h_
#define __koStore_h_

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QIODevice>
#include <QtCore/QStack>
#include <QtCore/QByteArray>
#include "koodf_export.h"

#include <kurl.h>

class QWidget;
class KoStorePrivate;

/**
 * Saves and loads KOffice documents using various backends. Currently supported
 * backends are ZIP, tar and directory.
 * We call a "store" the file on the hard disk (the one the users sees)
 * and call a "file" a file inside the store.
 */
class KOODF_EXPORT KoStore
{
public:

    enum Mode { Read, Write };
    enum Backend { Auto, Tar, Zip, Directory, Encrypted };

    /**
     * Open a store (i.e. the representation on disk of a KOffice document).
     *
     * @param fileName the name of the file to open
     * @param mode if KoStore::Read, open an existing store to read it.
     *             if KoStore::Write, create or replace a store.
     * @param backend the backend to use for the data storage.
     * Auto means automatically-determined for reading,
     * and the current format (now Zip) for writing.
     *
     * @param appIdentification the application's mimetype,
     * to be written in the file for "mime-magic" identification.
     * Only meaningful if mode is Write, and if backend!=Directory.
     */
    static KoStore *createStore(const QString &fileName, Mode mode, const QByteArray &appIdentification = "", Backend backend = Auto);

    /**
     * Create a store for any kind of QIODevice: file, memory buffer...
     * KoStore will take care of opening the QIODevice.
     * This method doesn't support the Directory store!
     */
    static KoStore *createStore(QIODevice *device, Mode mode, const QByteArray &appIdentification = "", Backend backend = Auto);

    /**
     * Open a store (i.e. the representation on disk of a KOffice document).
     *
     * @param window associated window (for the progress bar dialog and authentication)
     * @param url URL of the file to open
     * @param mode if KoStore::Read, open an existing store to read it.
     *             if KoStore::Write, create or replace a store.
     * @param backend the backend to use for the data storage.
     * Auto means automatically-determined for reading,
     * and the current format (now Zip) for writing.
     *
     * @param appIdentification the application's mimetype,
     * to be written in the file for "mime-magic" identification.
     * Only meaningful if mode is Write, and if backend!=Directory.
     *
     * If the file is remote, the backend Directory cannot be used!
     *
     * @bug saving not completely implemented (fixed temporary file)
     */
    static KoStore *createStore(QWidget *window, const KUrl &url, Mode mode, const QByteArray &appIdentification = "", Backend backend = Auto);

    /**
     * Destroys the store (i.e. closes the file on the hard disk)
     */
    virtual ~KoStore();

    /**
     * Returns the url of the store. It can be a filename or a remote url.
     * it can also be empty, if the store is a bytearray
     * @return the url of the store as supplied in the createStore calls
     */
    KUrl urlOfStore() const;

    /**
     * Open a new file inside the store
     * @param name The filename, internal representation ("root", "tar:/0"... ).
     *        If the tar:/ prefix is missing it's assumed to be a relative URI.
     * @return true on success.
     */
    bool open(const QString &name);

    /**
     * Check whether a file inside the store is currently opened with open(),
     * ready to be read or written.
     * @return true if a file is currently opened.
     */
    bool isOpen() const;

    /**
     * Close the file inside the store
     * @return true on success.
     */
    bool close();

    /**
     * Get a device for reading a file from the store directly
     * (slightly faster than read() calls)
     * You need to call @ref open first, and @ref close afterwards.
     */
    QIODevice *device() const;

    /**
     * Read data from the currently opened file. You can also use the streams
     * for this.
     */
    QByteArray read(qint64 max);

    /**
     * Write data into the currently opened file. You can also use the streams
     * for this.
     */
    qint64 write(const QByteArray &data);

    /**
     * Read data from the currently opened file. You can also use the streams
     * for this.
     * @return size of data read, -1 on error
     */
    qint64 read(char *buffer, qint64 length);

    /**
     * Write data into the currently opened file. You can also use the streams
     * for this.
     */
    virtual qint64 write(const char* data, qint64 length);

    /**
     * @return the size of the currently opened file, -1 on error.
     * Can be used as an argument for the read methods, for instance
     */
    qint64 size() const;

    /**
     * @return true if an error occurred
     */
    bool bad() const;

    /**
     * @return the mode used when opening, read or write
     */
    Mode mode() const;

    /**
     * Enters one or multiple directories. In Read mode this actually
     * checks whether the specified directories exist and returns false
     * if they don't. In Write mode we don't create the directory, we
     * just use the "current directory" to generate the absolute path
     * if you pass a relative path (one not starting with tar:/) when
     * opening a stream.
     * Note: Operates on internal names
     */
    bool enterDirectory(const QString &directory);

    /**
     * Leaves a directory. Equivalent to "cd .."
     * @return true on success, false if we were at the root already to
     * make it possible to "loop to the root"
     */
    bool leaveDirectory();

    /**
     * Returns the current path including a trailing slash.
     * Note: Returns a path in "internal name" style
     */
    QString currentPath() const;

    /**
     * Returns the current directory.
     * Note: Returns a path in "internal name" style
     */
    QString currentDirectory() const;

    /**
     * Stacks the current directory. Restore the current path using
     * @ref popDirectory .
     */
    void pushDirectory();

    /**
     * Restores the previously pushed directory. No-op if the stack is
     * empty.
     */
    void popDirectory();

    /**
     * @return true if the given file exists in the current directory,
     * i.e. if open(fileName) will work.
     */
    bool hasFile(const QString &fileName) const;

    /**
     * Imports a local file into a store
     * @param fileName file on hard disk
     * @param destName file in the store
     */
    bool addLocalFile(const QString &fileName, const QString &destName);

    /**
     * Imports data into a store
     * @param buffer data
     * @param destName file in the store
     */
    bool addDataToFile(QByteArray &buffer, const QString &destName);

    /**
     * Extracts a file out of the store
     * @param sourceName file in the store
     * @param fileName file on a disk
     */
    bool extractFile(const QString &sourceName, const QString &fileName);

    /**
     * Extracts a file out of the store to a buffer
     * @param sourceName file in the store
     * @param data memory buffer
     */
    bool extractFile(const QString &sourceName, QByteArray &data);

    //@{
    /// See QIODevice
    bool seek(qint64 pos);
    qint64 pos() const;
    bool atEnd() const;
    //@}

    /**
     * Do not expand file and directory names
     * Useful when using KoStore on non-KOffice files.
     * (This method should be called just after the constructor)
     */
    void disallowNameExpansion();

    /**
     * Call this before destroying the store, to be able to catch errors
     * (e.g. from ksavefile)
     */
    bool finalize();

    /**
     * Sets the password to be used for decryption or encryption of the store.
     * Use of this function is optional: an encryptable store should make
     * a best effort in obtaining a password if it wasn't supplied.
     *
     * This method only works before opening a file. It might fail when a file
     * has already been opened before calling this method.
     *
     * This method will not function for any store that is not encrypted or
     * can't be encrypted when saving.
     *
     * @param   password    A non-empty password.
     *
     * @return  True if the password was set.
     */
    virtual bool setPassword(const QString &password);

    /**
     * Retrieves the password used to encrypt or decrypt the store. Note that
     * QString() will returned if no password has been given or the store is
     * not encrypted.
     *
     * @return  The password this store is encrypted with.
     */
    virtual QString password();

    /**
     * Returns whether a store opened for reading is encrypted or a store opened
     * for saving will be encrypted.
     *
     * @return  True if the store is encrypted.
     */
    virtual bool isEncrypted();

    /**
     * Allow to enable or disable compression of the files. Only supported by the
     * ZIP backend.
     */
    virtual void setCompressionEnabled(bool e);
protected:

    KoStore();

    /**
     * Init store - called by constructor.
     * @return true on success
     */
    virtual bool init(Mode mode);

    /**
     * Finalize store - called by finalize.
     * @return true on success
     */
    virtual bool doFinalize() {
        return true;
    }

    /**
     * Open the file @p name in the store, for writing
     * On success, this method must set m_stream to a stream in which we can write.
     * @param name "absolute path" (in the archive) to the file to open
     * @return true on success
     */
    virtual bool openWrite(const QString &name) = 0;
    /**
     * Open the file @p name in the store, for reading.
     * On success, this method must set m_stream to a stream from which we can read,
     * as well as setting m_iSize to the size of the file.
     * @param name "absolute path" (in the archive) to the file to open
     * @return true on success
     */
    virtual bool openRead(const QString &name) = 0;

    /**
     * @return true on success
     */
    virtual bool closeRead() = 0;
    /**
     * @return true on success
     */
    virtual bool closeWrite() = 0;

    /**
     * Enter a subdirectory of the current directory.
     * The directory might not exist yet in Write mode.
     */
    virtual bool enterRelativeDirectory(const QString &dirName) = 0;
    /**
     * Enter a directory where we've been before.
     * It is guaranteed to always exist.
     */
    virtual bool enterAbsoluteDirectory(const QString &path) = 0;

    /**
     * Check if a file exists inside the store.
     * @param absPath the absolute path inside the store, i.e. not relative to the current directory
     */
    virtual bool fileExists(const QString &absPath) const = 0;

protected:
    KoStorePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoStore)

private:
    KoStore(const KoStore& store);    ///< don't copy
    KoStore& operator=(const KoStore& store);    ///< don't assign
};

#endif
