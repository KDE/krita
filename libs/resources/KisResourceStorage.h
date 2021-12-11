/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISRESOURCESTORAGE_H
#define KISRESOURCESTORAGE_H

#include <QSharedPointer>
#include <QScopedPointer>
#include <QString>
#include <QDateTime>
#include <QMap>

#include <KoResource.h>
#include <KisTag.h>

#include <klocalizedstring.h>

#include <kritaresources_export.h>

class KisStoragePlugin;

class KisStoragePluginFactoryBase
{
public:
    virtual ~KisStoragePluginFactoryBase(){}
    virtual KisStoragePlugin *create(const QString &/*location*/) { return 0; }
};

template<typename T>
class KisStoragePluginFactory : public KisStoragePluginFactoryBase
{
public:
    KisStoragePlugin *create(const QString &location) override {
        return new T(location);
    }
};

class KisResourceStorage;
typedef QSharedPointer<KisResourceStorage> KisResourceStorageSP;


/**
 * The KisResourceStorage class is the base class for
 * places where resources can be stored. Examples are
 * folders, bundles or Adobe resource libraries like
 * ABR files.
 */
class KRITARESOURCES_EXPORT KisResourceStorage
{
public:

    /// A resource item is simply an entry in the storage,
    struct ResourceItem {

        virtual ~ResourceItem() {}
        QString url;
        QString folder;
        QString resourceType;
        QDateTime lastModified;
    };

    class KRITARESOURCES_EXPORT TagIterator
    {
    public:
        virtual ~TagIterator() {}
        virtual bool hasNext() const = 0;
        /// The iterator is only valid if next() has been called at least once.
        virtual void next() = 0;

        /// A tag object on which we can set properties and which we can save
        virtual KisTagSP tag() const = 0;
    };

    class KRITARESOURCES_EXPORT ResourceIterator
    {
    public:

        virtual ~ResourceIterator() {}

        virtual bool hasNext() const = 0;
        /// The iterator is only valid if next() has been called at least once.
        virtual void next() = 0;

        virtual QString url() const = 0;
        virtual QString type() const = 0;
        virtual QDateTime lastModified() const = 0;
        virtual int guessedVersion() const { return 0; }
        virtual QSharedPointer<KisResourceStorage::ResourceIterator> versions() const;

        KoResourceSP resource() const;

    protected:
        /// This only loads the resource when called
        virtual KoResourceSP resourceImpl() const = 0;

    private:
        mutable KoResourceSP m_cachedResource;
        mutable QString m_cachedResourceUrl;
    };

    enum class StorageType : int {
        Unknown = 1,
        Folder = 2,
        Bundle = 3,
        AdobeBrushLibrary = 4,
        AdobeStyleLibrary = 5,
        Memory = 6
    };

    static QString storageTypeToString(StorageType storageType) {
        switch (storageType) {
        case StorageType::Unknown:
            return i18n("Unknown");
        case StorageType::Folder:
            return i18n("Folder");
        case StorageType::Bundle:
            return i18n("Bundle");
        case StorageType::AdobeBrushLibrary:
            return i18n("Adobe Brush Library");
        case StorageType::AdobeStyleLibrary:
            return i18n("Adobe Style Library");
        case StorageType::Memory:
            return i18n("Memory");
        default:
            return i18n("Invalid");
        }
    }


    static QString storageTypeToUntranslatedString(StorageType storageType) {
        switch (storageType) {
        case StorageType::Unknown:
            return ("Unknown");
        case StorageType::Folder:
            return ("Folder");
        case StorageType::Bundle:
            return ("Bundle");
        case StorageType::AdobeBrushLibrary:
            return ("Adobe Brush Library");
        case StorageType::AdobeStyleLibrary:
            return ("Adobe Style Library");
        case StorageType::Memory:
            return ("Memory");
        default:
            return ("Invalid");
        }
    }


    KisResourceStorage(const QString &location);
    ~KisResourceStorage();
    KisResourceStorage(const KisResourceStorage &rhs);
    KisResourceStorage &operator=(const KisResourceStorage &rhs);
    KisResourceStorageSP clone() const;

    /// The filename of the storage if it's a bundle or Adobe Library. This can
    /// also be empty (for the folder storage) or "memory" for the storage for
    /// temporary resources, a UUID for storages associated with documents.
    QString name() const;

    /// The absolute location of the storage
    QString location() const;

    /// true if the storage exists and can be used
    bool valid() const;

    /// The type of the storage
    StorageType type() const;

    /// The icond for the storage
    QImage thumbnail() const;

    /// The time and date when the storage was last modified, or created
    /// for memory storages.
    QDateTime timestamp() const;

    /// The time and date when the resource was last modified
    /// For filestorage
    QDateTime timeStampForResource(const QString &resourceType, const QString &filename) const;

    /// And entry in the storage; this is not the loaded resource
    ResourceItem resourceItem(const QString &url);

    /// The loaded resource for an entry in the storage
    KoResourceSP resource(const QString &url);

    /// The MD5 checksum of the resource in the storage
    QString resourceMd5(const QString &url);

    /// An iterator over all the resources in the storage
    QSharedPointer<ResourceIterator> resources(const QString &resourceType) const;

    /// An iterator over all the tags in the resource
    QSharedPointer<TagIterator> tags(const QString &resourceType) const;

    /// Adds a tag to the storage, however, it does not store the links between
    /// tags and resources.
    bool addTag(const QString &resourceType, KisTagSP tag);

    /// Creates a new version of the given resource.
    bool saveAsNewVersion(KoResourceSP resource);

    /// Adds the given resource to the storage. If there is already a resource
    /// with the given filename of the given type, this should return false and
    /// saveAsnewVersion should be used.
    bool addResource(KoResourceSP resource);

    /**
     * Copies the given file into this storage. Implementations should not overwrite
     * an existing resource with the same filename, but return false.
     *
     * @param url is the URL of the resource inside the storage, which is usually
     *            resource_type/resource_filename.ext
     */
    bool importResource(const QString &url, QIODevice *device);

    /**
     * Copies the given given resource from the storage into \p device
     *
     * @param url is the URL of the resource inside the storage, which is usually
     *            resource_type/resource_filename.ext
     */
    bool exportResource(const QString &url, QIODevice *device);

    /// Returns true if the storage supports versioning of the resources.
    /// It enables loadVersionedResource() call.
    bool supportsVersioning() const;

    /// Reloads the given resource from the persistent storage
    bool loadVersionedResource(KoResourceSP resource);

    static const QString s_meta_generator;
    static const QString s_meta_author;
    static const QString s_meta_title;
    static const QString s_meta_description;
    static const QString s_meta_initial_creator;
    static const QString s_meta_creator;
    static const QString s_meta_creation_date;
    static const QString s_meta_dc_date;
    static const QString s_meta_user_defined;
    static const QString s_meta_name;
    static const QString s_meta_value;
    static const QString s_meta_version;
    static const QString s_meta_license;
    static const QString s_meta_email;
    static const QString s_meta_website;

    void setMetaData(const QString &key, const QVariant &value);
    QStringList metaDataKeys() const;
    QVariant metaData(const QString &key) const;

private:

    friend class KisStorageModel;
    friend class KisResourceLocator;
    friend class KisResourceCacheDb;

    void setStorageId(int storageId);
    int storageId();

    class Private;
    QScopedPointer<Private> d;
};



inline QDebug operator<<(QDebug dbg, const KisResourceStorageSP storage)
{
    if (storage.isNull()) {
        dbg.nospace() << "[RESOURCESTORAGE] NULL";
    }
    else {
        dbg.nospace() << "[RESOURCESTORAGE] Name: " << storage->name()
                      << " Version: " << storage->location()
                      << " Valid: " << storage->valid()
                      << " Storage: " << KisResourceStorage::storageTypeToString(storage->type())
                      << " Timestamp: " << storage->timestamp()
                      << " Pointer: " << storage.data();
    }
    return dbg.space();
}

class KRITARESOURCES_EXPORT KisStoragePluginRegistry {
public:
    KisStoragePluginRegistry();
    virtual ~KisStoragePluginRegistry();

    void addStoragePluginFactory(KisResourceStorage::StorageType storageType, KisStoragePluginFactoryBase *factory);
    static KisStoragePluginRegistry *instance();
private:
    friend class KisResourceStorage;
    QMap<KisResourceStorage::StorageType, KisStoragePluginFactoryBase*> m_storageFactoryMap;

};

struct VersionedResourceEntry
{
    QString resourceType;
    QString filename;
    QList<QString> tagList;
    QDateTime lastModified;
    int guessedVersion = -1;
    QString guessedKey;

    struct KeyVersionLess {
        bool operator()(const VersionedResourceEntry &lhs, const VersionedResourceEntry &rhs) const {
            return lhs.guessedKey < rhs.guessedKey ||
                (lhs.guessedKey == rhs.guessedKey && lhs.guessedVersion < rhs.guessedVersion);
        }
    };

    struct KeyLess {
        bool operator()(const VersionedResourceEntry &lhs, const VersionedResourceEntry &rhs) const {
            return lhs.guessedKey < rhs.guessedKey;
        }
    };
};

class KRITARESOURCES_EXPORT KisStorageVersioningHelper {
public:

    static bool addVersionedResource(const QString &saveLocation, KoResourceSP resource, int minVersion);
    static QString chooseUniqueName(KoResourceSP resource,
                                    int minVersion,
                                    std::function<bool(QString)> checkExists);

    static void detectFileVersions(QVector<VersionedResourceEntry> &allFiles);


};

class KisVersionedStorageIterator : public KisResourceStorage::ResourceIterator
{
public:
    KisVersionedStorageIterator(const QVector<VersionedResourceEntry> &entries,
                                KisStoragePlugin *_q);

    bool hasNext() const override;
    void next() override;
    QString url() const override;
    QString type() const override;
    QDateTime lastModified() const override;
    KoResourceSP resourceImpl() const override;

    int guessedVersion() const override;

    QSharedPointer<KisResourceStorage::ResourceIterator> versions() const override;

protected:
    KisVersionedStorageIterator(const QVector<VersionedResourceEntry> &entries,
                                QVector<VersionedResourceEntry>::const_iterator begin,
                                QVector<VersionedResourceEntry>::const_iterator end,
                                KisStoragePlugin *_q);
protected:
    KisStoragePlugin *q = 0;
    const QVector<VersionedResourceEntry> m_entries;
    QVector<VersionedResourceEntry>::const_iterator m_it;
    QVector<VersionedResourceEntry>::const_iterator m_chunkStart;
    QVector<VersionedResourceEntry>::const_iterator m_begin;
    QVector<VersionedResourceEntry>::const_iterator m_end;
    bool m_isStarted = false;
};


#endif // KISRESOURCESTORAGE_H
