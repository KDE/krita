/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGBACKEND_H
#define KCONFIGBACKEND_H

#include <QObject>
#include <QString>
#include <QExplicitlySharedDataPointer>

#include <kconfigbase.h>
class KConfigBackendPrivate;

class KEntryMap;
class QFile;
class QByteArray;
class QIODevice;

/**
 * \class KConfigBackend kconfigbackend_p.h <KConfigBackend>
 *
 * Provides the implementation for accessing configuration sources.
 *
 * KConfig only provides an INI backend, but this class can be used
 * to create plugins that allow access to other file formats and
 * configuration systems.
 *
 * \internal
 */
class KConfigBackend : public QObject, public QSharedData
{
    Q_OBJECT

public:
    /**
     * Creates a new KConfig backend.
     *
     * If no @p system is given, or the given @p system is unknown, this method tries
     * to determine the correct backend to use.
     *
     * @param fileName      the absolute file name of the configuration file
     * @param system        the configuration system to use
     * @return a KConfigBackend object to be used with KConfig
     */
    static QExplicitlySharedDataPointer<KConfigBackend> create(const QString &fileName = QString());

    /**
     * Registers mappings from directories/files to configuration systems
     *
     * Allows you to tell KConfigBackend that create() should use a particular
     * backend for a particular file or directory.
     *
     * @warning currently does nothing
     *
     * @param entryMap the KEntryMap to build the mappings from
     */
    static void registerMappings(const KEntryMap &entryMap);

    /** Destroys the backend */
    virtual ~KConfigBackend();

    /** Allows the behaviour of parseConfig() to be tuned */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4 /// entries are allowed to be marked as @em expandable
    };
    Q_FLAG(ParseOption)
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** Allows the behaviour of writeConfig() to be tuned */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    Q_FLAG(WriteOption)
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig() */
    enum ParseInfo {
        ParseOk, /// the configuration was opened read/write
        ParseImmutable, /// the configuration is @em immutable
        ParseOpenError /// the configuration could not be opened
    };

    /**
     * Read persistent storage
     *
     * @param locale the locale to read entries for (if the backend supports localized entries)
     * @param pWriteBackMap the KEntryMap where the entries are placed
     * @param options See ParseOptions
     * @return See ParseInfo
     */
    virtual ParseInfo parseConfig(const QByteArray &locale,
                                  KEntryMap &pWriteBackMap,
                                  ParseOptions options = ParseOptions()) = 0;

    /**
     * Write the @em dirty entries to permanent storage
     *
     * @param locale the locale to write entries for (if the backend supports localized entries)
     * @param entryMap the KEntryMap containing the config object's entries.
     * @param options See WriteOptions
     *
     * @return @c true if the write was successful, @c false if writing the configuration failed
     */
    virtual bool writeConfig(const QByteArray &locale, KEntryMap &entryMap,
                             WriteOptions options) = 0;

    /**
     * If isWritable() returns false, writeConfig() will always fail.
     *
     * @return @c true if the configuration is writable, @c false if it is immutable
     */
    virtual bool isWritable() const = 0;
    /**
     * When isWritable() returns @c false, return an error message to
     * explain to the user why saving configuration will not work.
     *
     * The return value when isWritable() returns @c true is undefined.
     *
     * @returns a translated user-visible explanation for the configuration
     *          object not being writable
     */
    virtual QString nonWritableErrorMessage() const = 0;
    /**
     * @return the read/write status of the configuration object
     *
     * @see KConfigBase::AccessMode
     */
    virtual KConfigBase::AccessMode accessMode() const = 0;
    /**
     * Create the enclosing object of the configuration object
     *
     * For example, if the configuration object is a file, this should create
     * the parent directory.
     */
    virtual void createEnclosing() = 0;

    /**
     * Set the file path.
     *
     * @note @p path @b MUST be @em absolute.
     *
     * @param path the absolute file path
     */
    virtual void setFilePath(const QString &path) = 0;

    /**
     * Lock the file
     */
    virtual bool lock() = 0;
    /**
     * Release the lock on the file
     */
    virtual void unlock() = 0;
    /**
     * @return @c true if the file is locked, @c false if it is not locked
     */
    virtual bool isLocked() const = 0;

    /** @return the absolute path to the object */
    QString filePath() const;

protected:
    KConfigBackend();
    void setLocalFilePath(const QString &file);
    void setLocalIODevice(QIODevice &io);

private:
    KConfigBackendPrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBackend::ParseOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBackend::WriteOptions)

#if 0 // TODO re-enable if the plugin loading code is re-enabled
/**
 * Register a KConfig backend when it is contained in a loadable module
 */
#define K_EXPORT_KCONFIGBACKEND(libname, classname) \
    K_PLUGIN_FACTORY(factory, registerPlugin<classname>();)
#endif

#endif // KCONFIGBACKEND_H
